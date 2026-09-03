#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "event.h"
#include "test_framework.h"

void test_event_init(void)
{
    int fd;
    event_shut();
    fd = event_getfd();
    TEST_ASSERT_EQ(fd, -1);
    TEST_ASSERT_EQ(event_init(), 0);
    fd = event_getfd();
    TEST_ASSERT(fd >= 0);
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
}

void test_event_init_idempotent(void)
{
    int fd1;
    int fd2;
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    fd1 = event_getfd();
    TEST_ASSERT(fd1 >= 0);
    TEST_ASSERT_EQ(event_init(), 0);
    fd2 = event_getfd();
    TEST_ASSERT_EQ(fd1, fd2);
    event_shut();
}

void test_event_notify(void)
{
    int fd;
    char ch = '\0';
    ssize_t ret;
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    fd = event_getfd();
    TEST_ASSERT(fd >= 0);
    event_notify();
    ret = read(fd, &ch, 1);
    TEST_ASSERT_EQ(ret, 1);
    TEST_ASSERT_EQ(ch, 'x');
    event_shut();
}

void test_event_shut(void)
{
    TEST_ASSERT_EQ(event_init(), 0);
    TEST_ASSERT(event_getfd() >= 0);
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
    event_shut();
    TEST_ASSERT_EQ(event_getfd(), -1);
}

void test_event_close_in_child(void)
{
    pid_t pid;
    int status;
    event_shut();
    TEST_ASSERT_EQ(event_init(), 0);
    TEST_ASSERT(event_getfd() >= 0);
    pid = fork();
    TEST_ASSERT(pid >= 0);

    if (pid < 0)
    {
        event_shut();
        return;
    }

    if (pid == 0)
    {
        event_close_in_child();
        if (event_getfd() == -1)
            _exit(0);
        _exit(1);
    }

    TEST_ASSERT_EQ(waitpid(pid, &status, 0), pid);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    TEST_ASSERT(event_getfd() >= 0);
    event_shut();
}
