#include "trash.h"

char *substr(const char *str, const size_t start, size_t length) {
    size_t i = 0;
    size_t j = 0;

    if (start >= strlen(str))
        return strdup("");
    if (length > strlen(str) - start)
        length = strlen(str) - start;
    char* substr = malloc(length + 1);
    if (!substr)
        return NULL;
    while (str[i] && length > 0) {
        if (i >= start) {
            substr[j] = str[i];
            length--;
            j++;
        }
        i++;
    }
    substr[j] = '\0';
    return substr;
}

char *append_char(char *str, const char c) {
    size_t len = 0;

    if (str)
        len = strlen(str);
    char* new_str = malloc(len + 2);
    if (!new_str)
        return NULL;
    if (str)
        memcpy(new_str, str, len);
    new_str[len] = c;
    new_str[len + 1] = '\0';
    free(str);
    return new_str;
}

char *append_str(char *dst, const char *src) {
    size_t len_dst = 0;
    size_t len_src = 0;

    if (dst)
        len_dst = strlen(dst);
    if (src)
        len_src = strlen(src);
    char *new_str = malloc(len_dst + len_src + 1);
    if (!new_str)
        return NULL;
    if (dst)
        memcpy(new_str, dst, len_dst);
    if (src)
        memcpy(new_str + len_dst, src, len_src);
    new_str[len_dst + len_src] = '\0';
    free(dst);
    return new_str;
}

char *itoa(const int n) {
    long num = n;
    long tmp = num;
    int len = (num <= 0) ? 1 : 0;

    while (tmp != 0) {
        tmp /= 10;
        len++;
    }
    char *str = malloc(len + 1);
    if (!str)
        return NULL;
    str[len] = '\0';
    if (num == 0) {
        str[0] = '0';
        return str;
    }
    if (num < 0) {
        num = -num;
        str[0] = '-';
    }
    while (num > 0) {
        str[--len] = (char)((num % 10) + '0');
        num /= 10;
    }
    return str;
}

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

void clean_up(shell_t *shell) {
    if (!shell)
        return;
    if (shell->input) {
        free(shell->input);
        shell->input = NULL;
    }
    if (shell->tokens) {
        free_tokens(shell->tokens);
        shell->tokens = NULL;
    }
    if (shell->ast) {
        free_ast(shell->ast);
        shell->ast = NULL;
    }
}
