#define _GNU_SOURCE
#include "sig.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "event.h"


static void sigchld_handler(int sig);
static void sigint_handler(int sig);
static void sigtstp_handler(int sig);

void signal_init(ShellContext *ctx)
{
    (void)ctx;
    struct sigaction sasa = {0};
    sasa.sa_handler = sigint_handler;
    sigemptyset(&sasa.sa_mask);
    sasa.sa_flags = 0;
    if (sigaction(SIGINT, &sasa, NULL) < 0)
        perror("sigaction");
    struct sigaction sastp = {0};
    sastp.sa_handler = sigtstp_handler;
    sigemptyset(&sastp.sa_mask);
    sastp.sa_flags = 0;
    if (sigaction(SIGTSTP, &sastp, NULL) < 0)
    perror("sigaction");

    struct sigaction sa = {0};
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
        perror("sigaction");
}

void signal_reset_child(void)
{
    struct sigaction sasagei = {0};
    sasagei.sa_handler = SIG_DFL;
    sigemptyset(&sasagei.sa_mask);
    sasagei.sa_flags = 0;
    if (sigaction(SIGINT, &sasagei, NULL) < 0)
        perror("sigaction");
    if (sigaction(SIGTSTP, &sasagei, NULL) < 0)
        perror("sigaction");
}

static void sigchld_handler(int sig)
{
    (void)sig;
    event_notify();
}

static void sigint_handler(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}
static void sigtstp_handler(int sig)
{
    (void)sig;
}
