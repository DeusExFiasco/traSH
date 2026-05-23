#ifndef MINISHELL_MINISHELL_H
#define MINISHELL_MINISHELL_H

// ReSharper disable CppUnusedIncludeDirective
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "tokenizer.h"
#include "parser.h"

typedef struct shell {
    int last_status;
    char **env;
    char *input;
} shell_t;

token_t *tokenize_input(const shell_t *shell);
ast_node_t *parse_tokens(token_t *tokens);
void print_tokens(const token_t *tokens);
void print_ast(const ast_node_t *node);

#endif //MINISHELL_MINISHELL_H
