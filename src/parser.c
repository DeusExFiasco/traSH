#include "minishell.h"

static token_type_t current_type(token_t **current) {
    const token_t *t = *current;
    return t ? t->type : TK_EOF;
}

static token_t *current_token(token_t **current) {
    return *current;
}

static void advance(token_t **current) {
    if (*current)
        *current = (*current)->next;
}

static ast_node_t *new_binary_node(ast_type_t type, ast_node_t *left, ast_node_t *right) {
    ast_node_t *node  = malloc(sizeof(ast_node_t));
    if (!node)
        return nullptr;
    node->node_type = type;
    node->args = nullptr;
    node->redirs = nullptr;
    node->left = left;
    node->right = right;
    return node;
}

static ast_node_t *new_command_node(char **args, redirection_t *redirs) {
    ast_node_t *node  = malloc(sizeof(ast_node_t));
    if (!node)
        return nullptr;
    node->node_type = AST_COMMAND;
    node->args = args;
    node->redirs = redirs;
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

static redirection_t *new_redir(token_type_t redir_type, char *target) {
    redirection_t *redir = malloc(sizeof(redirection_t));
    if (!redir)
        return nullptr;
    redir->redir_type = redir_type;
    redir->target = strdup(target);
    redir->heredoc_fd = -1;
    redir->next = nullptr;
    return redir;
}

static ast_node_t *parse_command(token_t **current, shell_t *shell) {
    token_t *token = current_token(current);
    char **argv = nullptr;
    size_t argc = 0;
    redirection_t *redirs = nullptr;
    redirection_t *redirs_tail = nullptr;

    while (token && token->type != TK_EOF) {
        if (token->type == TK_WORD) {
            char *val = strdup(token->value ? token->value : "");
            if (!val)
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
            char **new_argv = realloc(argv, sizeof(char *) * (argc + 2));
            if (!new_argv) {
                free(val);
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
            }
            argv = new_argv;
            argv[argc++] = val;
            argv[argc] = nullptr;
            advance(current);
            token = current_token(current);
        } else if (token->type == TK_REDIR_IN || token->type == TK_REDIR_OUT ||
            token->type == TK_APPEND || token->type == TK_HEREDOC) {
            const token_type_t redir_type = token->type;
            advance(current);
            token = current_token(current);
            if (!token || token->type != TK_WORD) {
                handle_error(SYNTAX_ERROR, token ? token->value : nullptr, shell);
                return nullptr;
            }
            redirection_t *redirection = new_redir(redir_type, token->value);
            if (!redirection)
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
            if (!redirs)
                redirs = redirs_tail = redirection;
            else {
                redirs_tail->next = redirection;
                redirs_tail = redirection;
            }
            advance(current);
            token = current_token(current);
        }

        if (!token || token->type == TK_PIPE || token->type == TK_AND || token->type == TK_OR ||
            token->type == TK_SEMICOLON || token->type == TK_EOF) {
            break;
        }
    }
    return new_command_node(argv, redirs);
}

static ast_node_t *parse_pipe(token_t **current, shell_t *shell) {
    ast_node_t *left = parse_command(current, shell);
    if (!left)
        return nullptr;
    while (current_type(current) == TK_PIPE) {
        advance(current);
        ast_node_t *right = parse_command(current, shell);
        if (!right) {
            free_ast(left);
            return nullptr;
        }
        left = new_binary_node(AST_PIPE, left, right);
    }
    return left;
}

static ast_node_t *parse_logical(token_t **current, shell_t *shell) {
    ast_node_t *left = parse_pipe(current, shell);
    if (!left)
        return nullptr;
    while (current_type(current) == TK_AND || current_type(current) == TK_OR) {
        const token_type_t token_type = current_type(current);
        advance(current);
        ast_node_t *right = parse_pipe(current, shell);
        if (!right) {
            free_ast(left);
            return nullptr;
        }
        left = new_binary_node(token_type == TK_AND ? AST_AND : AST_OR, left, right);
    }
    return left;
}

static ast_node_t *parse_sequence(token_t **current, shell_t *shell) {
    ast_node_t *left = parse_logical(current, shell);
    if (!left)
        return nullptr;
    while (current_type(current) == TK_SEMICOLON) {
        advance(current);
        ast_node_t *right = parse_logical(current, shell);
        if (!right) {
            free_ast(left);
            return nullptr;
        }
        left = new_binary_node(AST_SEMICOLON, left, right);
    }
    return left;
}

ast_node_t *parse_tokens(shell_t *shell) {
    token_t *current = shell->tokens;
    ast_node_t *root = parse_sequence(&current, shell);
    (void)current;
    return root;
}
