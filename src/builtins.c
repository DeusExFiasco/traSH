#include "minishell.h"

int builtin_cd(char **argv, shell_t *shell) {
    const char *path = argv[1];
    if (chdir(path) != 0) {
        handle_error(INVALID_PATH, argv[1], shell);
        return shell->last_status;
    }
    return 0;
}

static bool is_numeric(const char *s) {
    if (!s || !*s)
        return false;
    if (*s == '+' || *s == '-')
        s++;
    if (!*s)
        return false;
    while (*s) {
        if (!isdigit((unsigned char)*s))
            return false;
        s++;
    }
    return true;
}

int builtin_exit(char **argv, shell_t *shell) {
    int code = 0;
    if (!argv[1]) {
        code = shell->last_status;
        clean_up(shell);
        exit(code);
    }
    if (argv[2] || (!is_numeric(argv[1]))) {
        handle_error(INVALID_INPUT, argv[0], shell);
        return shell->last_status;
    }
    code = atoi(argv[1]) & 0xFF;
    clean_up(shell);
    exit(code);
}

static bool print_exported(char **env) {
    if (!env)
        return false;
    char **copy = dup_env(env);
    if (!copy)
        return false;
    int count = 0;
    while (copy[count])
        count++;
    // Bubble Sort
    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (strcmp(copy[i], copy[j]) > 0) {
                char *tmp = copy[i];
                copy[i] = copy[j];
                copy[j] = tmp;
            }
        }
    }
    for (int i = 0; i < count; ++i) {
        char *eq = strchr(copy[i], '=');
        if (!eq)
            printf("declare -x %s\n", copy[i]);
        else {
            *eq = '\0';
            printf("declare -x %s=\"%s\"\n", copy[i], eq + 1);
            *eq = '=';
        }
        free(copy[i]);
    }
    free(copy);
    return true;
}

int builtin_export(char **argv, shell_t *shell) {
    if (!argv[1]) {
        if (!print_exported(shell->env))
            handle_fatal_error(MEMORY_ERROR, nullptr, shell);
        return 0;
    }

    int status = 0;
    for (int i = 1; argv[i]; ++i) {
        char *arg = argv[i];
        char *plus = strstr(arg, "+=");
        char *eq = strstr(arg, "=");
        if (plus && (!eq || plus > eq))
            plus = nullptr;
        if (plus) {
            *plus = '\0';
            if (!is_valid_identifier(arg)) {
                handle_error(INVALID_INPUT, argv[0], shell);
                status = 1;
            } else if (!append_env(&shell->env, arg, plus + 2))
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
            *plus = '+';
            continue;
        }
        if (eq) {
            *eq = '\0';
            if (!is_valid_identifier(arg)) {
                handle_error(INVALID_INPUT, argv[0], shell);
                status = 1;
            } else if (!set_env(&shell->env, arg, eq + 1))
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
            *eq = '=';
        } else {
            if (!is_valid_identifier(arg)) {
                handle_error(INVALID_INPUT, argv[0], shell);
                status = 1;
            } else if (!set_env(&shell->env, arg, ""))
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
        }
    }
    return status;
}

int builtin_unset(char **argv, shell_t *shell) {
    for (int i = 1; argv[i]; ++i)
        unset_env(&shell->env, argv[i]);
    return 0;
}
