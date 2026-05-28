#include "trash.h"

static void run_line(shell_t *shell, const char *line) {
    shell->input = strdup(line);
    if (!shell->input)
        handle_fatal_error(MEMORY_ERROR, NULL, shell);
    shell->tokens = tokenize_input(shell);
    if (shell->tokens) {
        shell->ast = parse_tokens(shell);
        if (shell->ast)
            shell->last_status = execute(shell);
    }
    clean_up(shell);
}

static void run_batch(shell_t *shell) {
    char *line = NULL;
    ssize_t n;
    size_t size = 0;
    while ((n = getline(&line, &size, stdin)) != -1) {
        if (n > 0 && line[n - 1] == '\n')
            line[n - 1] = '\0';
        if (line[0] == '\0')
            continue;
        run_line(shell, line);
    }
    free(line);
}

static char *make_cool_prompt(const shell_t *shell) {
    char hostname[256];
    char *user = get_env_var(strdup("USER"), shell->env);
    char *pwd = get_env_var(strdup("PWD"), shell->env);

    if (gethostname(hostname, sizeof(hostname)) != 0)
        strcpy(hostname, "unknown");
    if (!user)
        user = strdup("user");
    if (!pwd)
        pwd = strdup("pwd");
    size_t user_len = strlen(user);
    size_t host_len = strlen(hostname);
    size_t pwd_len = strlen(pwd);
    size_t color_teal_len = strlen(CLR_TEAL);
    size_t color_red_len  = strlen(CLR_RED);
    size_t color_reset_len = strlen(CLR_RESET);
    size_t total_len = user_len + host_len + pwd_len
                    + color_teal_len * 3
                    + color_red_len * 3
                    + color_reset_len
                    + 9;
    char* cool_prompt = malloc(total_len);
    if (!cool_prompt) {
        free(user);
        free(pwd);
        return NULL;
    }
    snprintf(cool_prompt, total_len,
        CLR_TEAL "%s" CLR_RED "@"
        CLR_TEAL "%s" CLR_RED "⟫"
        CLR_TEAL "%s" CLR_RED "⟫ " CLR_RESET,
        user, hostname, pwd);
    free(user);
    free(pwd);
    return cool_prompt;
}

static void shell_loop(shell_t *shell) {
    while (true) {
        char *prompt = make_cool_prompt(shell);
        shell->input = readline(prompt ? prompt : "$ ");
        free(prompt);
        if (shell->input == NULL)
            break;
        if (interrupted())
            shell->last_status = 130;
        if (shell->input[0] == '\0')
            continue;
        add_history(shell->input);
        shell->tokens = tokenize_input(shell);
        if (shell->tokens) {
#if DEBUG
            print_tokens(shell->tokens);
#endif
            shell->ast = parse_tokens(shell);
            if (shell->ast) {
#if DEBUG
                print_ast(shell->ast);
#endif
                shell->last_status = execute(shell);
            }
        }
        clean_up(shell);
    }
}

int main(const int argc, char **argv, char **env) {
    shell_t shell = {0};
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    shell.last_status = 0;
    shell.env = dup_env(env);
    if (!shell.env)
        handle_fatal_error(ENV_NOT_FOUND, NULL, &shell);
    if (argc > 1) {
        run_line(&shell, argv[1]);
        free_env(shell.env);
        return shell.last_status;
    }
    if (!isatty(STDIN_FILENO)) {
        run_batch(&shell);
        free_env(shell.env);
        return shell.last_status;
    }
    shell_loop(&shell);
    free_env(shell.env);
    return shell.last_status;
}
