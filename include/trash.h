#ifndef TRASH_H
#define TRASH_H

#define _POSIX_C_SOURCE 200809L

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
#include <readline/history.h>

#include "tokenizer.h"
#include "parser.h"

#define DEBUG false

#define CLR_RESET "\001\033[0m\002"
#define CLR_TEAL  "\001\033[38;5;51m\002"
#define CLR_RED   "\001\033[38;5;203m\002"

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

void handle_sigint(int lmao);
bool interrupted();

token_t *tokenize_input(shell_t *shell);
ast_node_t *parse_tokens(shell_t *shell);
int execute(shell_t *shell);

int builtin_cd(char **argv, shell_t *shell);
int builtin_exit(char **argv, shell_t *shell);
int builtin_export(char **argv, shell_t *shell);
int builtin_unset(char **argv, shell_t *shell);

char **dup_env(char **envp);
void free_env(char **env);
bool is_valid_identifier(const char *str);
char *get_env_var(char *name, char **env);
bool set_env(char ***envp, const char *name, const char *value);
int unset_env(char ***envp, const char *name);
bool append_env(char ***envp, const char *name, const char *suffix);

void clean_up(shell_t *shell);

void free_tokens(token_t *tokens);
void free_ast(ast_node_t *node);
void handle_error(error_t error, char *context, shell_t *shell);
void handle_fatal_error(error_t error, char *context, shell_t *shell);

#if DEBUG
void print_tokens(const token_t *tokens);
void print_ast(const ast_node_t *node);
#endif

#endif //TRASH_H
