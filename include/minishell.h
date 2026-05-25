#ifndef MINISHELL_MINISHELL_H
#define MINISHELL_MINISHELL_H

// ReSharper disable CppUnusedIncludeDirective
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <readline/readline.h>

#include "tokenizer.h"
#include "parser.h"

typedef struct shell {
    int last_status;
    char **env;
    char *input;
    token_t *tokens;
    ast_node_t *ast;
} shell_t;

typedef enum error {
    INVALID_INPUT,
    SYNTAX_ERROR,
    COMMAND_NOT_FOUND,
    INVALID_PATH,
    PERMISSION_DENIED,
    MEMORY_ERROR,
    PIPE_FAIL,
    FORK_FAIL,
    EXEC_FAIL,
    ENV_NOT_FOUND
} error_t;

token_t *tokenize_input(shell_t *shell);
ast_node_t *parse_tokens(shell_t *shell);
int execute(shell_t *shell);

char *get_env_var(char *name, char **env);
void clean_up(shell_t *shell);

void free_tokens(token_t *tokens);
void free_ast(ast_node_t *node);
void handle_error(error_t error, char *context, shell_t *shell);
void handle_fatal_error(error_t error, char *context, shell_t *shell);

void print_tokens(const token_t *tokens);
void print_ast(const ast_node_t *node);

#endif //MINISHELL_MINISHELL_H
