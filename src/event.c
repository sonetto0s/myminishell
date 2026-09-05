#include "event.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static int event_pipe[2] = {-1, -1};

static int event_set_flags(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) return -1;

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;

    flags = fcntl(fd, F_GETFD);
    if (flags < 0) return -1;

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) return -1;

    return 0;
}

int event_init(void)
{
    if (event_pipe[0] >= 0 && event_pipe[1] >= 0) return 0;

    if (event_pipe[0] >= 0 || event_pipe[1] >= 0) event_shut();

    if (pipe(event_pipe) < 0) {
        perror("pipe");
        event_pipe[0] = -1;
        event_pipe[1] = -1;
        return -1;
    }

    if (event_set_flags(event_pipe[0]) < 0 || event_set_flags(event_pipe[1]) < 0) {
        perror("fcntl");
        event_shut();
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
    if (event_pipe[1] < 0) return;

    char value = 'x';

    while (1) {
        ssize_t ret = write(event_pipe[1], &value, 1);

        if (ret == 1) return;
        if (ret < 0 && errno == EINTR) continue;
        if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;

        return;
    }
}

int event_drain(void)
{
    if (event_pipe[0] < 0) return -1;

    char buffer[64];

    while (1) {
        ssize_t ret = read(event_pipe[0], buffer, sizeof(buffer));

        if (ret > 0) continue;
        if (ret == 0) return -1;
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;

        return -1;
    }
}

void event_shut(void)
{
    if (event_pipe[1] >= 0) {
        int fd = event_pipe[1];
        event_pipe[1] = -1;
        close(fd);
    }

    if (event_pipe[0] >= 0) {
        int fd = event_pipe[0];
        event_pipe[0] = -1;
        close(fd);
    }
}
void event_close_in_child(void)
{
    event_shut();
}


