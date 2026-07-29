#define _GNU_SOURCE
#include "sig.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include "job.h"


static void sigchld_handler(int sig);
static void sigint_handler(int sig);

static ShellContextStatus *shellctx = NULL;

void signal_init(ShellContextStatus *ctx)
{
    shellctx = ctx;
    struct sigaction sasa = {0};
    sasa.sa_handler = sigint_handler;
    sigemptyset(&sasa.sa_mask);
    sasa.sa_flags = 0;
    if (sigaction(SIGINT, &sasa, NULL) < 0)
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
}

static void sigchld_handler(int sig)
{
    (void)sig;
    int status = 0;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (shellctx)
            job_remove(&shellctx->jobs, pid);
    }
}

static void sigint_handler(int sig)
{
    (void)sig;
    printf("\n");
}