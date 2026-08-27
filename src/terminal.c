#include "terminal.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

static pid_t shell_pgid;

static void terminal_ignore_signals()
{
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

int terminal_init(void)
{
    terminal_ignore_signals();
    if (!isatty(STDIN_FILENO))
    {
        return 0;
    }
    pid_t pid = getpid();
    shell_pgid = pid;
    if (setpgid(pid, pid) < 0)
    {
        perror("setpgid");
        return -1;
    }
    if (tcsetpgrp(STDIN_FILENO, pid) < 0)
    {
        perror("tcsetpgrp");
        return -1;
    }
    return 0;
}

int terminal_set_foreground(pid_t pgid)
{
    return tcsetpgrp(STDIN_FILENO, pgid);
}

int terminal_restore()
{
    return tcsetpgrp(STDIN_FILENO, shell_pgid);
}
