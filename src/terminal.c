#include "terminal.h"
#include "log.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static pid_t shell_pgid = -1;
static int terminal_fd = -1;
static int terminal_initialized = 0;
static int shell_modes_valid = 0;
static struct termios shell_modes;

static int terminal_set_pgrp(pid_t pgid)
{
    while (tcsetpgrp(terminal_fd, pgid) < 0) {
        if (errno == EINTR) continue;
        return -1;
    }

    return 0;
}

int terminal_init(void)
{
    terminal_initialized = 0;
    terminal_fd = -1;
    shell_pgid = -1;
    shell_modes_valid = 0;

    if (!isatty(STDIN_FILENO)) return 0;

    terminal_fd = open("/dev/tty", O_RDWR | O_CLOEXEC);

    if (terminal_fd < 0)
        return -1;

    pid_t shell_group = getpgrp();

    while (1) {
        pid_t foreground = tcgetpgrp(terminal_fd);

        if (foreground < 0)
            goto fail;

        if (foreground == shell_group)
            break;

        if (kill(-shell_group, SIGTTIN) < 0) {
            if (errno == EINTR)
                continue;

            goto fail;
        }

        shell_group = getpgrp();
    }

    sigset_t block_set;
    sigset_t old_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGTTOU);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0)
        goto fail;

    pid_t pid = getpid();

    if (shell_group != pid) {
        if (setpgid(0, 0) < 0 && errno != EACCES) {
            sigprocmask(SIG_SETMASK, &old_set, NULL);
            goto fail;
        }

        shell_group = getpgrp();
    }

    shell_pgid = shell_group;

    if (terminal_set_pgrp(shell_pgid) < 0) {
        sigprocmask(SIG_SETMASK, &old_set, NULL);
        goto fail;
    }

    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0)
        goto fail;

    if (tcgetattr(terminal_fd, &shell_modes) < 0)
        goto fail;

    shell_modes_valid = 1;
    terminal_initialized = 1;

    return 0;

fail:
    close(terminal_fd);
    terminal_fd = -1;
    shell_pgid = -1;
    shell_modes_valid = 0;

    return -1;
}

void terminal_shutdown(void)
{
    if (terminal_initialized && terminal_restore() < 0)
        log_error("failed restore terminal during shutdown");

    if (terminal_fd >= 0)
        close(terminal_fd);

    terminal_fd = -1;
    shell_pgid = -1;
    shell_modes_valid = 0;
    terminal_initialized = 0;
}

int terminal_is_initialized(void)
{
    return terminal_initialized;
}

int terminal_set_foreground(pid_t pgid)
{
    if (!terminal_initialized) return 0;
    if (pgid <= 0) return -1;

    return terminal_set_pgrp(pgid);
}

int terminal_get_modes(struct termios *modes)
{
    if (!terminal_initialized) return 0;
    if (!modes) return -1;

    while (tcgetattr(terminal_fd, modes) < 0) {
        if (errno == EINTR) continue;
        return -1;
    }

    return 0;
}

int terminal_set_modes(const struct termios *modes)
{
    if (!terminal_initialized) return 0;
    if (!modes) return -1;

    while (tcsetattr(terminal_fd, TCSADRAIN, modes) < 0) {
        if (errno == EINTR) continue;
        return -1;
    }

    return 0;
}

int terminal_restore(void)
{
    if (!terminal_initialized) return 0;
    if (shell_pgid <= 0) return -1;

    if (terminal_set_pgrp(shell_pgid) < 0)
        return -1;

    if (shell_modes_valid && terminal_set_modes(&shell_modes) < 0)
        return -1;

    return 0;
}
