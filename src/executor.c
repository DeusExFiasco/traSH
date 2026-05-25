#include "minishell.h"

static int execute_node(ast_node_t *node, shell_t *shell);

static char *join_path(const char *dir, const char *cmd) {
    const size_t len_dir = strlen(dir);
    const size_t len_cmd = strlen(cmd);
    char *full_path = malloc(len_dir + 1 + len_cmd + 1);
    if (!full_path)
        return nullptr;
    memcpy(full_path, dir, len_dir);
    full_path[len_dir] = '/';
    memcpy(full_path + len_dir + 1, cmd, len_cmd);
    full_path[len_dir + 1 + len_cmd] = '\0';
    return full_path;
}

static char *resolve_command(const char *cmd, char **env) {
    if (!cmd || cmd[0] == '\0')
        return nullptr;
    if (strchr(cmd, '/')) {
        if (access(cmd, X_OK) == 0)
            return strdup(cmd);
        return nullptr;
    }
    char *paths = get_env_var(strdup("PATH"), env);
    if (!paths)
        return nullptr;
    char *saveptr = nullptr;
    for (char *path = strtok_r(paths, ":", &saveptr); path; path = strtok_r(nullptr, ":", &saveptr)) {
        char *candidate = join_path(path, cmd);
        if (!candidate)
            continue;
        if (access(candidate, X_OK) == 0) {
            free(paths);
            return candidate;
        }
        free(candidate);
    }
    free(paths);
    return nullptr;
}

static int wait_status(const pid_t pid) {
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return status;
}

static int create_heredoc(const char *eof) {
    int fds[2];
    if (pipe(fds) < 0)
        return -1;
    while (true) {
        char *line = readline(" > ");
        if (!line)
            break;
        if (strcmp(line, eof) == 0) {
            free(line);
            break;
        }
        write(fds[1], line, strlen(line));
        write(fds[1], "\n", 1);
        free(line);
    }
    close(fds[1]);
    return fds[0];
}

static bool apply_redirections(redirection_t *redir, shell_t *shell) {
    while (redir) {
        int fd = -1;
        if (redir->redir_type == TK_REDIR_IN)
            fd = open(redir->target, O_RDONLY);
        else if (redir->redir_type == TK_REDIR_OUT)
            fd = open(redir->target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        else if (redir->redir_type == TK_APPEND)
            fd = open(redir->target, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else if (redir->redir_type == TK_HEREDOC) { // FIXME: HEREDOC before PIPE freezes shell!
            int heredoc_fd = create_heredoc(redir->target);
            if (heredoc_fd < 0) {
                handle_error(INVALID_PATH, redir->target, shell);
                return false;
            }
            dup2(heredoc_fd, STDIN_FILENO);
            close(heredoc_fd);
            redir = redir->next;
            continue;
        }
        if (fd < 0) {
            handle_error(INVALID_PATH, redir->target, shell);
            return false;
        }
        if (redir->redir_type == TK_REDIR_IN)
            dup2(fd, STDIN_FILENO);
        else
            dup2(fd, STDOUT_FILENO);
        close(fd);
        redir = redir->next;
    }
    return true;
}

static int exec_child(ast_node_t *node, shell_t *shell) {
    if (!apply_redirections(node->redirs, shell))
        return shell->last_status;
    char *path = resolve_command(node->args[0], shell->env);
    if (!path) {
        if (strchr(node->args[0], '/'))
            handle_error(INVALID_PATH, node->args[0], shell);
        else
            handle_error(COMMAND_NOT_FOUND, node->args[0], shell);
        return shell->last_status;
    }
    execve(path, node->args, shell->env);
    if (errno == EACCES)
        handle_error(PERMISSION_DENIED, node->args[0], shell);
    else if (errno == ENOENT)
        handle_error(INVALID_PATH, node->args[0], shell);
    else
        handle_error(EXEC_FAIL, node->args[0], shell);
    free(path);
    return shell->last_status;
}

static int exec_command(ast_node_t *node, shell_t *shell) {
    if (!node->args || !node->args[0])
        return 0;
    const pid_t pid = fork();
    if (pid < 0)
        handle_fatal_error(FORK_FAIL, nullptr, shell);
    if (pid == 0)
        exit(exec_child(node, shell));
    return wait_status(pid);
}

static int exec_pipe(const ast_node_t *node, shell_t *shell) {
    int fds[2];
    if (pipe(fds) < 0)
        handle_fatal_error(PIPE_FAIL, nullptr, shell);
    const pid_t left_pid = fork();
    if (left_pid < 0)
        handle_fatal_error(FORK_FAIL, nullptr, shell);
    if (left_pid == 0) {
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        exit(exec_child(node->left, shell));
    }
    const pid_t right_pid = fork();
    if (right_pid < 0)
        handle_fatal_error(FORK_FAIL, nullptr, shell);
    if (right_pid == 0) {
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        exit(exec_child(node->right, shell));
    }
    close(fds[0]);
    close(fds[1]);
    wait_status(left_pid);
    return wait_status(right_pid);
}

static int execute_node(ast_node_t *node, shell_t *shell) {
    if (!node)
        return 0;
    switch (node->node_type) {
    case AST_COMMAND:
        return exec_command(node, shell);
    case AST_PIPE:
        return exec_pipe(node, shell);
    case AST_AND: {
        const int left = execute_node(node->left, shell);
        if (left == 0)
            return execute_node(node->right, shell);
        return left;
    }
    case AST_OR: {
        const int left = execute_node(node->left, shell);
        if (left != 0)
            return execute_node(node->right, shell);
        return left;
    }
    case AST_SEMICOLON:
        execute_node(node->left, shell);
        return execute_node(node->right, shell);
    default:
        return 1;
    }
}

int execute(shell_t *shell) {
    ast_node_t *root = shell->ast;
    return execute_node(root, shell);
}
