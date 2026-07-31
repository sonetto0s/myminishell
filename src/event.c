#include "event.h"
#include <stdio.h>
#include <unistd.h>

static int event_pipe[2];

int event_init(void)
{
    if (pipe(event_pipe) < 0)
    {
        perror("pipe");
        return -1;
    }
    return 0;
}

int event_getfd(void)
{
    return event_pipe[0];
}

void event_notify(void)
{
    char notice_msg = 'x';
    write(event_pipe[1], &notice_msg, 1);
}

void event_shut(void)
{
    close(event_pipe[0]);
    close(event_pipe[1]);
}
