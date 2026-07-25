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
