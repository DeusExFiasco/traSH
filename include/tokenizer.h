#ifndef MINISHELL_TOKENIZER_H
#define MINISHELL_TOKENIZER_H

// ReSharper disable CppUnusedIncludeDirective

typedef enum token_type {
    TK_WORD, // any string
    TK_PIPE, // |
    TK_REDIR_IN, // <
    TK_REDIR_OUT, // >
    TK_APPEND, // >>
    TK_HEREDOC, // <<
    TK_AND, // &&
    TK_OR, // ||
    TK_SEMICOLON, // ;
    TK_EOF
} token_type_t;

typedef struct token {
    token_type_t type;
    char *value;
    struct token *next;
} token_t;

typedef enum quote_state {
    QUOTE_NONE,
    QUOTE_SINGLE,
    QUOTE_DOUBLE
} quote_state_t;

// Utils
char *substr(const char *str, size_t start, size_t length);
char *append_char(char *str, char c);
char *append_str(char *dst, const char *src);
char *itoa(int n);

#endif //MINISHELL_TOKENIZER_H
