#include "minishell.h"

static void shell_loop(shell_t *shell) {
    while (true) {
        shell->input = readline("$ ");
        if (shell->input == NULL)
            break;
        if (interrupted())
            shell->last_status = 130;
        if (shell->input[0] == '\0')
            continue;
        add_history(shell->input);
        shell->tokens = tokenize_input(shell);
        if (shell->tokens) {
            //print_tokens(shell->tokens);
            shell->ast = parse_tokens(shell);
            if (shell->ast) {
                //print_ast(shell->ast);
                shell->last_status = execute(shell);
            }
        }
        clean_up(shell);
    }
}

int main(const int argc, char **argv, char **env) {
    (void)argc, (void)argv;
    shell_t shell = {0};
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    shell.last_status = 0;
    shell.env = dup_env(env);
    if (!shell.env)
        handle_fatal_error(ENV_NOT_FOUND, nullptr, &shell);
    shell_loop(&shell);
    return shell.last_status;
}
