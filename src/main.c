#include "minishell.h"

static void shell_loop(shell_t *shell) {
    while (true) {
        shell->input = readline("$ ");

        if (shell->input == NULL) {
            break;
        }
        if (shell->input[0] == '\0') {
            continue;
        }
        token_t *tokens = tokenize_input(shell);
        print_tokens(tokens);
        ast_node_t *ast = parse_tokens(tokens);
        print_ast(ast);
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
