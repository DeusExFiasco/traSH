#include "minishell.h"

void free_tokens(token_t *tokens) {
    while (tokens) {
        token_t *next = tokens->next;
        free(tokens->value);
        free(tokens);
        tokens = next;
    }
}

void free_ast(ast_node_t *node) {
    if (!node)
        return;
    if (node->node_type == AST_COMMAND) {
        if (node->args) {
            for (int i = 0; node->args[i]; ++i)
                free(node->args[i]);
            free(node->args);
        }
        while (node->redirs) {
            redirection_t *next = node->redirs->next;
            free(node->redirs->target);
            free(node->redirs);
            node->redirs = next;
        }
    } else {
        free_ast(node->left);
        free_ast(node->right);
    }
    free(node);
}

static void clean_exit(shell_t *shell) {
    if (!shell)
        return;
    if (shell->input) {
        free(shell->input);
        shell->input = nullptr;
    }
    if (shell->tokens) {
        free_tokens(shell->tokens);
        shell->tokens = nullptr;
    }
    if (shell->ast) {
        free_ast(shell->ast);
        shell->tokens = nullptr;
    }
    exit(shell->last_status);
}

static const char *get_error_message(const error_t error) {
    static const char *messages[] = {
        [INVALID_INPUT] = "Invalid input",
        [SYNTAX_ERROR] = "Parse error",
        [COMMAND_NOT_FOUND] = "Command not found",
        [INVALID_PATH] = "No such file or directory",
        [PERMISSION_DENIED] = "Permission denied",
        [MEMORY_ERROR] = "Memory allocation failed",
        [PIPE_FAIL] = "Pipe failure",
        [FORK_FAIL] = "Fork failure",
        [EXEC_FAIL] = "Failed to execute command",
        [ENV_NOT_FOUND] = "Environment variables not found"
    };
    return messages[error];
}

static int get_error_status(const error_t error) {
    static constexpr int statuses[] = {
        [INVALID_INPUT] = 1,
        [SYNTAX_ERROR] = 2,
        [COMMAND_NOT_FOUND] = 127,
        [INVALID_PATH] = 1,
        [PERMISSION_DENIED] = 126,
        [MEMORY_ERROR] = 1,
        [PIPE_FAIL] = 1,
        [FORK_FAIL] = 1,
        [EXEC_FAIL] = 126,
        [ENV_NOT_FOUND] = 1
    };
    return statuses[error];
}

void handle_error(const error_t error, char *context, shell_t *shell) {
    fprintf(stderr, "minishell: ");
    if (context)
        fprintf(stderr, "%s: ", context);
    fprintf(stderr, "%s\n", get_error_message(error));
    shell->last_status = get_error_status(error);
}

void handle_fatal_error(const error_t error, char *context, shell_t *shell) {
    handle_error(error, context, shell);
    clean_exit(shell);
}
