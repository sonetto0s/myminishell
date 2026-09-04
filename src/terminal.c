#include "terminal.h"
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static pid_t shell_pgid = -1;
static int terminal_initialized = 0;

static int terminal_set_pgrp(pid_t pgid)
{
    while (1) {
        if (tcsetpgrp(STDIN_FILENO, pgid) == 0) return 0;
        if (errno == EINTR) continue;
        return -1;
    }
}

int terminal_init(void)
{
    terminal_initialized = 0;
    shell_pgid = -1;

    if (!isatty(STDIN_FILENO)) return 0;

    pid_t pid = getpid();
    pid_t pgrp = getpgrp();

    if (pgrp != pid) {
        if (setpgid(0, 0) < 0) {
            perror("setpgid");
            return -1;
        }
    }

    shell_pgid = getpgrp();

    if (shell_pgid <= 0) return -1;

    if (terminal_set_pgrp(shell_pgid) < 0) {
        perror("tcsetpgrp");
        return -1;
    }

    terminal_initialized = 1;
    return 0;
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

int terminal_restore(void)
{
    if (!terminal_initialized) return 0;
    if (shell_pgid <= 0) return -1;

    return terminal_set_pgrp(shell_pgid);
}


