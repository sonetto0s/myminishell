#include "sig.h"
#include "event.h"
#include <errno.h>
#include <signal.h>
#include <string.h>

static volatile sig_atomic_t pending_events = SIGNAL_EVENT_NONE;

static void signal_handler(int sig)
{
    int saved_errno = errno;

    if (sig == SIGCHLD) pending_events |= SIGNAL_EVENT_CHILD;
    else if (sig == SIGINT) pending_events |= SIGNAL_EVENT_INTERRUPT;

    event_notify();
    errno = saved_errno;
}

static int install_handler(int sig, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGCHLD);

    sa.sa_flags = 0;

    return sigaction(sig, &sa, NULL);
}

static int ignore_signal(int sig)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);

    return sigaction(sig, &sa, NULL);
}

int signal_init(void)
{
    pending_events = SIGNAL_EVENT_NONE;

    if (install_handler(SIGINT, signal_handler) < 0) return -1;
    if (install_handler(SIGCHLD, signal_handler) < 0) return -1;

    if (ignore_signal(SIGQUIT) < 0) return -1;
    if (ignore_signal(SIGTSTP) < 0) return -1;
    if (ignore_signal(SIGTTIN) < 0) return -1;
    if (ignore_signal(SIGTTOU) < 0) return -1;

    return 0;
}

int signal_take_events(void)
{
    sigset_t block_set;
    sigset_t old_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    sigaddset(&block_set, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) return SIGNAL_EVENT_NONE;

    int events = pending_events;
    pending_events = SIGNAL_EVENT_NONE;

    sigprocmask(SIG_SETMASK, &old_set, NULL);

    return events;
}

void signal_reset_child(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    sigset_t empty_set;
    sigemptyset(&empty_set);
    sigprocmask(SIG_SETMASK, &empty_set, NULL);
}



