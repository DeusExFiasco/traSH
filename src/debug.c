#include "trash.h"

const char *token_to_str(const token_t token) {
    switch (token.type) {
    case TK_WORD: return "WORD";
    case TK_PIPE: return "PIPE";
    case TK_REDIR_IN: return "REDIR_IN";
    case TK_REDIR_OUT: return "REDIR_OUT";
    case TK_APPEND: return "APPEND";
    case TK_HEREDOC: return "HEREDOC";
    case TK_OR: return "OR";
    case TK_AND: return "AND";
    case TK_SEMICOLON: return "SEMICOLON";
    default: return "EOF";
    }
}

void print_tokens(const token_t *tokens) {
    const token_t *temp = tokens;

    while (temp) {
        printf("%s ", token_to_str(*temp));
        temp = temp->next;
    }
    printf("\n");
}

static const char *ast_type_to_str(ast_type_t type) {
    switch (type) {
    case AST_COMMAND: return "AST_COMMAND";
    case AST_PIPE: return "AST_PIPE";
    case AST_AND: return "AST_AND";
    case AST_OR: return "AST_OR";
    case AST_SEMICOLON: return "AST_SEMICOLON";
    default: return "AST_UNKNOWN";
    }
}

static const char *redir_type_to_str(token_type_t type) {
    switch (type) {
    case TK_REDIR_IN: return "<";
    case TK_REDIR_OUT: return ">";
    case TK_APPEND: return ">>";
    case TK_HEREDOC: return "<<";
    default: return "?";
    }
}

static void print_indent(int depth) {
    while (depth-- > 0)
        printf("  ");
}

static void print_args(char **args, int depth) {
    print_indent(depth);
    printf("args:");
    if (!args) {
        printf(" (none)\n");
        return;
    }
    for (size_t i = 0; args[i]; ++i)
        printf(" %s", args[i]);
    printf("\n");
}

static void print_redirs(const redirection_t *redir, int depth) {
    while (redir) {
        print_indent(depth);
        printf("redir: %s %s\n", redir_type_to_str(redir->redir_type),
               redir->target ? redir->target : "(null)");
        redir = redir->next;
    }
}

static void print_ast_rec(const ast_node_t *node, int depth) {
    if (!node) {
        print_indent(depth);
        printf("(null)\n");
        return;
    }

    print_indent(depth);
    printf("%s\n", ast_type_to_str(node->node_type));

    if (node->node_type == AST_COMMAND) {
        print_args(node->args, depth + 1);
        print_redirs(node->redirs, depth + 1);
        return;
    }

    print_indent(depth + 1);
    printf("left:\n");
    print_ast_rec(node->left, depth + 2);

    print_indent(depth + 1);
    printf("right:\n");
    print_ast_rec(node->right, depth + 2);
}

void print_ast(const ast_node_t *node) {
    print_ast_rec(node, 0);
}
