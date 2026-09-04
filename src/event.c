#include "event.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static int event_pipe[2] = {-1, -1};

int event_init(void)
{
    if (event_pipe[0] >= 0 && event_pipe[1] >= 0)
        return 0;

    if (event_pipe[0] >= 0 || event_pipe[1] >= 0)
        event_shut();

    if (pipe(event_pipe) < 0)
    {
        perror("pipe");
        event_pipe[0] = -1;
        event_pipe[1] = -1;
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
    if (event_pipe[1] < 0)
        return;

    char notice_msg = 'x';
    ssize_t ret;

    do
    {
        ret = write(event_pipe[1], &notice_msg, 1);
    }
    while (ret < 0 && errno == EINTR);

    if (ret < 0)
        perror("failed notify event");
}

void event_shut(void)
{
    if (event_pipe[0] >= 0)
    {
        close(event_pipe[0]);
        event_pipe[0] = -1;
    }

    if (event_pipe[1] >= 0)
    {
        close(event_pipe[1]);
        event_pipe[1] = -1;
    }
}

void event_close_in_child(void)
{
    event_shut();
}
