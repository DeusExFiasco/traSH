#include "minishell.h"

static volatile sig_atomic_t signal_received;

void handle_sigint(int lmao) {
    (void)lmao;
    signal_received = 1;
    rl_replace_line("", 0);
    write(STDERR_FILENO, "\n", 1);
    if (waitpid(-1, nullptr, WNOHANG) == 0)
        return;
    rl_on_new_line();
    rl_redisplay();
}

bool interrupted() {
    int was_interrupted = signal_received;
    signal_received = 0;
    return was_interrupted;
}
