#define _GNU_SOURCE
#include "sig.h"
#include <signal.h>
#include <stdio.h>
void signal_init(void)
{
    struct sigaction sasa={0};
    sasa.sa_handler = SIG_IGN;
    sigemptyset(&sasa.sa_mask);
    sasa.sa_flags = 0;
    if (sigaction(SIGINT, &sasa, NULL) < 0)
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