#include "minishell.h"

int builtin_cd(char **argv, shell_t *shell) {
    const char *path = argv[1];
    if (chdir(path) != 0) {
        handle_error(INVALID_PATH, argv[1], shell);
        return shell->last_status;
    }
    return 0;
}

int builtin_exit(char **argv, shell_t *shell) {
    int code = shell->last_status;
    if (argv[1])
        code = atoi(argv[1]) & 0xFF;
    shell->last_status = code;
    clean_up(shell);
    exit(shell->last_status);
}

// TODO: add export and unset!
