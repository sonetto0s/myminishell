#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "event.h"
#include "test_framework.h"
#include "sig.h"

void test_event_init(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
    TEST_ASSERT_EQ(event_init(), 0);
    int fd = event_getfd();
    TEST_ASSERT(fd >= 0);
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
}

void test_event_init_idempotent(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    int fd1 = event_getfd();
    TEST_ASSERT(fd1 >= 0);
    TEST_ASSERT_EQ(event_init(), 0);
    int fd2 = event_getfd();
    TEST_ASSERT_EQ(fd1, fd2);
    event_shut();
}

void test_event_notify(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    int fd = event_getfd();
    TEST_ASSERT(fd >= 0);
    event_notify();
    char ch = '\0';
    ssize_t ret = read(fd, &ch, 1);
    TEST_ASSERT_EQ(ret, 1);
    TEST_ASSERT_EQ(ch, 'x');
    event_shut();
}

void test_event_shut(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    TEST_ASSERT(event_getfd() >= 0);

    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);

    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
}

void test_event_fd_closed_after_shut(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    int fd = event_getfd();

    TEST_ASSERT(fd >= 0);
    event_shut();
    errno = 0;

    int ret = fcntl(fd, F_GETFD);
    TEST_ASSERT_EQ(ret, -1);
    TEST_ASSERT_EQ(errno, EBADF);
    TEST_ASSERT_EQ(event_getfd(), -1);
}

void test_event_reinit_after_shut(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    int first_fd = event_getfd();
    TEST_ASSERT(first_fd >= 0);
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
    TEST_ASSERT_EQ(event_init(), 0);
    int second_fd = event_getfd();
    TEST_ASSERT(second_fd >= 0);
    event_notify();
    char ch = '\0';
    ssize_t ret = read(second_fd, &ch, 1);
    TEST_ASSERT_EQ(ret, 1);
    TEST_ASSERT_EQ(ch, 'x');

    event_shut();
}

void test_event_notify_after_shut(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
    event_notify();
    TEST_ASSERT_EQ(event_getfd(), -1);
}

void test_event_repeated_lifecycle(void)
{
    event_shut();
    for (int i = 0; i < 32; i++)
    {
        TEST_ASSERT_EQ(event_init(), 0);
        int fd = event_getfd();
        TEST_ASSERT(fd >= 0);
        event_notify();
        char ch = '\0';
        ssize_t ret = read(fd, &ch, 1);
        TEST_ASSERT_EQ(ret, 1);
        TEST_ASSERT_EQ(ch, 'x');

        event_shut();
        TEST_ASSERT_EQ(event_getfd(), -1);

        errno = 0;
        ret = fcntl(fd, F_GETFD);
        TEST_ASSERT_EQ(ret, -1);
        TEST_ASSERT_EQ(errno, EBADF);
    }
}

void test_event_close_in_child(void)
{
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    int parent_fd = event_getfd();
    TEST_ASSERT(parent_fd >= 0);
    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0)
    {
        event_shut();
        return;
    }

    if (pid == 0)
    {
        event_close_in_child();

        if (event_getfd() != -1)
            _exit(1);

        _exit(0);
    }

    int status = 0;

    TEST_ASSERT_EQ(waitpid(pid, &status, 0), pid);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    TEST_ASSERT_EQ(event_getfd(), parent_fd);
    event_notify();
    char ch = '\0';
    ssize_t ret = read(parent_fd, &ch, 1);

    TEST_ASSERT_EQ(ret, 1);
    TEST_ASSERT_EQ(ch, 'x');
    event_shut();

}



void test_event_shutdown_signal_safety(void)
{
    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0) {
        if (event_init() < 0)
            _exit(1);

        if (signal_init() < 0)
            _exit(2);

        signal_shutdown();
        event_shut();

        raise(SIGINT);

        _exit(0);
    }

    int status = 0;

    TEST_ASSERT_EQ(waitpid(pid, &status, 0), pid);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
}











