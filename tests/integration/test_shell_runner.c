#include "test_framework.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *minishell_path(void)
{
    const char *path = getenv("MINISHELL_TEST_BIN");

    if (path && *path) return path;

    return "./build/default/minishell";
}

int shell_run_script(const char *script, char **output, int *status)
{
    int input_pipe[2];
    int output_pipe[2];

    if (!script || !output) return -1;

    *output = NULL;

    if (pipe(input_pipe) < 0) return -1;

    if (pipe(output_pipe) < 0) {
        close(input_pipe[0]);
        close(input_pipe[1]);
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        close(input_pipe[0]);
        close(input_pipe[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);
        return -1;
    }

    if (pid == 0) {
        if (dup2(input_pipe[0], STDIN_FILENO) < 0) _exit(127);
        if (dup2(output_pipe[1], STDOUT_FILENO) < 0) _exit(127);
        if (dup2(output_pipe[1], STDERR_FILENO) < 0) _exit(127);

        close(input_pipe[0]);
        close(input_pipe[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);

        execl(minishell_path(), "minishell", (char *)NULL);
        _exit(127);
    }

    close(input_pipe[0]);
    close(output_pipe[1]);

    size_t script_len = strlen(script);
    size_t written = 0;

    while (written < script_len) {
        ssize_t n = write(input_pipe[1], script + written, script_len - written);

        if (n < 0) {
            if (errno == EINTR) continue;

            close(input_pipe[1]);
            close(output_pipe[0]);
            waitpid(pid, NULL, 0);
            return -1;
        }

        written += (size_t)n;
    }

    close(input_pipe[1]);

    size_t capacity = 4096;
    size_t length = 0;
    char *buffer = malloc(capacity);

    if (!buffer) {
        close(output_pipe[0]);
        waitpid(pid, NULL, 0);
        return -1;
    }

    while (1) {
        if (length + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char *new_buffer = realloc(buffer, new_capacity);

            if (!new_buffer) {
                free(buffer);
                close(output_pipe[0]);
                waitpid(pid, NULL, 0);
                return -1;
            }

            buffer = new_buffer;
            capacity = new_capacity;
        }

        ssize_t n = read(output_pipe[0], buffer + length, capacity - length - 1);

        if (n < 0) {
            if (errno == EINTR) continue;

            free(buffer);
            close(output_pipe[0]);
            waitpid(pid, NULL, 0);
            return -1;
        }

        if (n == 0) break;

        length += (size_t)n;
    }

    close(output_pipe[0]);

    buffer[length] = '\0';
    *output = buffer;

    int child_status;

    while (waitpid(pid, &child_status, 0) < 0) {
        if (errno == EINTR) continue;

        free(buffer);
        *output = NULL;
        return -1;
    }

    if (status) *status = child_status;

    return 0;
}


