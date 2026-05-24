#ifndef MINISHELL_PARSER_H
#define MINISHELL_PARSER_H

#include "tokenizer.h"

typedef enum ast_type {
    AST_COMMAND,
    AST_PIPE,
    AST_AND,
    AST_OR,
    AST_SEMICOLON
} ast_type_t;

typedef struct redirection {
    token_type_t redir_type;
    char *target;
    struct redirection *next;
} redirection_t;

typedef struct ast_node {
    ast_type_t node_type;
    char **args;
    redirection_t *redirs;
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

#endif //MINISHELL_PARSER_H
