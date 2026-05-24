#include "minishell.h"

static void shell_loop(shell_t *shell) {
    while (true) {
        shell->input = readline("$ ");

        if (shell->input == NULL)
            break;
        if (shell->input[0] == '\0')
            continue;
        shell->tokens = tokenize_input(shell);
        print_tokens(shell->tokens);
        shell->ast = parse_tokens(shell);
        print_ast(shell->ast);
        // execute ast
        // loop back
    }
}

int main(const int argc, char **argv, char **env) {
    (void)argc, (void)argv;
    shell_t shell = {0};
    shell.last_status = 0;
    shell.env = env;
    shell_loop(&shell);
    return shell.last_status;
}
