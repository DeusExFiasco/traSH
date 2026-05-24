#include "minishell.h"

char *substr(const char *str, const size_t start, size_t length) {
    size_t i = 0;
    size_t j = 0;

    if (start >= strlen(str))
        return strdup("");
    if (length > strlen(str) - start)
        length = strlen(str) - start;
    char* substr = malloc(length + 1);
    if (!substr)
        return nullptr;
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
        return nullptr;
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
        return nullptr;
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
    int len = 1;

    while (tmp != 0) {
        tmp /= 10;
        len++;
    }
    char *str = malloc(len + 1);
    if (!str)
        return nullptr;
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
