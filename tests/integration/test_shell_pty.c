#define _XOPEN_SOURCE 600
#include "test_framework.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static const char *minishell_path(void)
{
    const char *path = getenv("MINISHELL_TEST_BIN");

    if (path && *path) return path;

    return "./build/default/minishell";
}

static int read_until_text(int fd, const char *target, char *output, size_t output_size, int timeout_ms)
{
    size_t length = 0;
    int elapsed = 0;

    if (output == NULL || output_size == 0)
        return -1;

    output[0] = '\0';
    while (elapsed < timeout_ms)
    {
        struct pollfd pfd =
        {
            .fd = fd,
            .events = POLLIN
        };

        int ret = poll(&pfd, 1, 50);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        elapsed += 50;

        if (ret == 0)
            continue;

        if (!(pfd.revents & (POLLIN | POLLHUP)))
            continue;

        if (length + 1 >= output_size)
            return -1;

        ssize_t n = read(fd, output + length, output_size - length - 1);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return -1;

        length += (size_t)n;
        output[length] = '\0';

        if (strstr(output, target) != NULL)
            return 0;
    }

    return -1;
}

static int wait_for_text(int fd, const char *target, int timeout_ms)
{
    char output[4096];
    return read_until_text(fd, target, output, sizeof(output), timeout_ms);
}

static int wait_for_prompt_with_text(int fd, const char *target, int timeout_ms)
{
    char output[4096];
    size_t length = 0;
    int elapsed = 0;

    if (!target) return -1;

    output[0] = '\0';

    while (elapsed < timeout_ms) {
        struct pollfd pfd = {
            .fd = fd,
            .events = POLLIN
        };

        int ret = poll(&pfd, 1, 50);

        if (ret < 0) {
            if (errno == EINTR) continue;
            return -1;
        }

        elapsed += 50;

        if (ret == 0) continue;
        if (!(pfd.revents & (POLLIN | POLLHUP))) continue;
        if (length + 1 >= sizeof(output)) return -1;

        ssize_t n = read(fd, output + length, sizeof(output) - length - 1);

        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }

        if (n == 0) return -1;

        length += (size_t)n;
        output[length] = '\0';

        char *target_pos = strstr(output, target);

        if (target_pos &&
            strstr(target_pos + strlen(target), ">>MiniShell ") != NULL) {
            return 0;
        }
    }

    return -1;
}

static int write_all(int fd, const char *text)
{
    size_t length = strlen(text);
    size_t written = 0;

    while (written < length)
    {
        ssize_t n = write(fd, text + written, length - written);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        written += (size_t)n;
    }

    return 0;
}

static int write_control(int fd, unsigned char control)
{
    while (1)
    {
        ssize_t n = write(fd, &control, 1);
        if (n == 1)
            return 0;

        if (n < 0 && errno == EINTR)
            continue;

        return -1;
    }
}

static int wait_for_pid(pid_t pid, int *status, int timeout_ms)
{
    int elapsed = 0;

    struct timespec delay =
  {
        .tv_sec = 0,
        .tv_nsec = 50000000L
    };

    while (elapsed < timeout_ms)
    {
        pid_t ret = waitpid(pid, status, WNOHANG);

        if (ret == pid)
            return 0;

        if (ret < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        nanosleep(&delay, NULL);
        elapsed += 50;
    }

    return -1;
}

static void kill_pty_session(pid_t shell_pid, pid_t supervisor_pid)
{
    if (shell_pid > 0)
    {
        kill(-shell_pid, SIGKILL);
        kill(shell_pid, SIGKILL);
    }

    if (supervisor_pid > 0)
    {
        kill(supervisor_pid, SIGKILL);

        while (waitpid(supervisor_pid, NULL, 0) < 0)
        {
            if (errno != EINTR)
                break;
        }
    }
}

static int start_pty_shell(int *master_fd, pid_t *supervisor_pid, pid_t *shell_pid)
{
    int pid_pipe[2];

    *master_fd = -1;
    *supervisor_pid = -1;
    *shell_pid = -1;

    int master = posix_openpt(O_RDWR | O_NOCTTY);

    if (master < 0)
        return -1;

    if (grantpt(master) < 0 || unlockpt(master) < 0)
    {
        close(master);
        return -1;
    }

    char *slave_name = ptsname(master);

    if (slave_name == NULL)
    {
        close(master);
        return -1;
    }

    if (pipe(pid_pipe) < 0)
    {
        close(master);
        return -1;
    }

    pid_t supervisor = fork();

    if (supervisor < 0)
    {
        close(pid_pipe[0]);
        close(pid_pipe[1]);
        close(master);
        return -1;
    }

    if (supervisor == 0)
    {
        close(pid_pipe[0]);
        close(master);

        if (setsid() < 0)
            _exit(127);

        int slave = open(slave_name, O_RDWR);

        if (slave < 0)
            _exit(127);

        if (ioctl(slave, TIOCSCTTY, 0) < 0)
        {
            close(slave);
            _exit(127);
        }

        pid_t shell = fork();

        if (shell < 0)
        {
            close(slave);
            _exit(127);
        }

        if (shell == 0)
        {
            close(pid_pipe[1]);

            if (dup2(slave, STDIN_FILENO) < 0)
                _exit(127);

            if (dup2(slave, STDOUT_FILENO) < 0)
                _exit(127);

            if (dup2(slave, STDERR_FILENO) < 0)
                _exit(127);

            if (slave > STDERR_FILENO)
                close(slave);

           execl(minishell_path(), "minishell", (char *)NULL);
            _exit(127);
        }

        close(slave);

        ssize_t sent;

        do
        {
            sent = write(pid_pipe[1], &shell, sizeof(shell));
        }
        while (sent < 0 && errno == EINTR);

        close(pid_pipe[1]);

        if (sent != (ssize_t)sizeof(shell))
        {
            kill(shell, SIGKILL);
            waitpid(shell, NULL, 0);
            _exit(127);
        }

        int status;

        while (waitpid(shell, &status, 0) < 0)
        {
            if (errno != EINTR)
                _exit(127);
        }

        if (WIFEXITED(status))
            _exit(WEXITSTATUS(status));

        if (WIFSIGNALED(status))
            _exit(128 + WTERMSIG(status));

        _exit(127);
    }

    close(pid_pipe[1]);

    pid_t shell;
    ssize_t received;

    do
    {
        received = read(pid_pipe[0], &shell, sizeof(shell));
    }
    while (received < 0 && errno == EINTR);

    close(pid_pipe[0]);

    if (received != (ssize_t)sizeof(shell))
    {
        kill_pty_session(-1, supervisor);
        close(master);
        return -1;
    }

    *master_fd = master;
    *supervisor_pid = supervisor;
    *shell_pid = shell;

    return 0;
}

void test_shell_pty_start_exit(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    TEST_ASSERT_EQ(ret, 0);

    int status = 0;

    ret = wait_for_pid(supervisor_pid, &status, 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0)
    {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
    else
    {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}

void test_shell_pty_ctrl_c(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "sleep 5\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 200000000L
    };

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 3);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    TEST_ASSERT_EQ(ret, 0);

    int status = 0;

    ret = wait_for_pid(supervisor_pid, &status, 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0)
    {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
    else
    {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}

void test_shell_pty_ctrl_z(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "sleep 5\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 200000000L
    };

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 26);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_prompt_with_text(master_fd, "now it is stopped", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "fg\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 3);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    TEST_ASSERT_EQ(ret, 0);

    int status = 0;

    ret = wait_for_pid(supervisor_pid, &status, 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0)
    {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
    else
    {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}

void test_shell_pty_bg(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "sleep 3\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 200000000L
    };

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 26);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_prompt_with_text(master_fd, "now it is stopped", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "bg\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_prompt_with_text(master_fd, "sleep &", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_prompt_with_text(master_fd, "now it is running", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, "exit 0", 4000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    TEST_ASSERT_EQ(ret, 0);

    int status = 0;

    ret = wait_for_pid(supervisor_pid, &status, 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0)
    {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
    else
    {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}

void test_shell_pty_fg(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "sleep 5 &\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_prompt_with_text(master_fd, "now it is running", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "fg\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 300);
    TEST_ASSERT_EQ(ret, -1);

    if (ret == 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 200000000L
    };

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 3);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    char output[4096];

    ret = read_until_text(master_fd, ">>MiniShell ", output, sizeof(output), 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    TEST_ASSERT(strstr(output, "sleep") == NULL);
    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }
    ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    TEST_ASSERT_EQ(ret, 0);

    int status = 0;
    ret = wait_for_pid(supervisor_pid, &status, 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0)
    {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
    else
    {
        kill_pty_session(shell_pid, supervisor_pid);
    }
    close(master_fd);
}

void test_shell_pty_prompt_ctrl_c(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);
    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_control(master_fd, 3);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "echo alive\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_prompt_with_text(master_fd, "alive", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    int status = 0;
    ret = wait_for_pid(supervisor_pid, &status, 2000);

    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    } else {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}

void test_shell_pty_pipeline_ctrl_z_fg(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);
    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "sleep 10 | sleep 10\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 200000000L
    };

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 26);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_prompt_with_text(master_fd, "now it is stopped", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "fg\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, ">>MiniShell ", 300);
    TEST_ASSERT_EQ(ret, -1);

    if (ret == 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    nanosleep(&delay, NULL);

    ret = write_control(master_fd, 3);
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    char output[4096];

    if (ret == 0) {
        ret = read_until_text(master_fd, ">>MiniShell ", output, sizeof(output), 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        TEST_ASSERT(strstr(output, "now it is running") == NULL &&
                    strstr(output, "now it is stopped") == NULL);
    }

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    int status = 0;
    ret = wait_for_pid(supervisor_pid, &status, 2000);

    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    } else {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}

void test_shell_pty_background_tty_stop(void)
{
    int master_fd;
    pid_t supervisor_pid;
    pid_t shell_pid;

    int ret = start_pty_shell(&master_fd, &supervisor_pid, &shell_pid);
    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    ret = wait_for_text(master_fd, ">>MiniShell ", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "cat &\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = wait_for_text(master_fd, "stopped by 21", 2000);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        kill_pty_session(shell_pid, supervisor_pid);
        close(master_fd);
        return;
    }

    ret = write_all(master_fd, "jobs\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_prompt_with_text(master_fd, "now it is stopped", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    ret = write_all(master_fd, "exit\n");
    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        ret = wait_for_text(master_fd, ">>MiniShell 已退出", 2000);
    }

    TEST_ASSERT_EQ(ret, 0);

    int status = 0;
    ret = wait_for_pid(supervisor_pid, &status, 2000);

    TEST_ASSERT_EQ(ret, 0);

    if (ret == 0) {
        TEST_ASSERT(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    } else {
        kill_pty_session(shell_pid, supervisor_pid);
    }

    close(master_fd);
}
