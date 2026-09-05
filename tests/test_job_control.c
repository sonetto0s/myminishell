#include "event.h"
#include "job.h"
#include "sig.h"
#include "test_framework.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_ms(long ms)
{
    struct timespec ts = {
        .tv_sec = ms / 1000,
        .tv_nsec = (ms % 1000) * 1000000L
    };

    while (nanosleep(&ts, &ts) < 0) {
        if (errno != EINTR) break;
    }
}

static int setpgid_parent(pid_t pid, pid_t pgid)
{
    while (setpgid(pid, pgid) < 0) {
        if (errno == EINTR) continue;
        if (errno == EACCES || errno == ESRCH) return 0;
        return -1;
    }

    return 0;
}

void test_event_nonblocking_cloexec(void)
{
    event_shut();

    TEST_ASSERT_EQ(event_init(), 0);

    int fd = event_getfd();
    TEST_ASSERT(fd >= 0);

    int flags = fcntl(fd, F_GETFL);
    TEST_ASSERT(flags >= 0);

    if (flags >= 0) {
        TEST_ASSERT((flags & O_NONBLOCK) != 0);
    }

    int fdflags = fcntl(fd, F_GETFD);
    TEST_ASSERT(fdflags >= 0);

    if (fdflags >= 0) {
        TEST_ASSERT((fdflags & FD_CLOEXEC) != 0);
    }

    event_shut();
}

void test_event_notify_flood(void)
{
    event_shut();

    TEST_ASSERT_EQ(event_init(), 0);

    for (int i = 0; i < 100000; i++) {
        event_notify();
    }

    TEST_ASSERT_EQ(event_drain(), 0);

    int fd = event_getfd();
    TEST_ASSERT(fd >= 0);

    char ch = '\0';

    errno = 0;
    ssize_t ret = read(fd, &ch, 1);

    TEST_ASSERT_EQ(ret, -1);
    TEST_ASSERT(errno == EAGAIN || errno == EWOULDBLOCK);

    event_shut();
}

void test_signal_event_delivery(void)
{
    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0) {
        event_shut();

        if (event_init() < 0) _exit(1);
        if (signal_init() < 0) _exit(2);

        if (raise(SIGINT) != 0) _exit(3);
        if (event_drain() < 0) _exit(4);

        int events = signal_take_events();

        if (!(events & SIGNAL_EVENT_INTERRUPT)) _exit(5);

        if (raise(SIGCHLD) != 0) _exit(6);
        if (event_drain() < 0) _exit(7);

        events = signal_take_events();

        if (!(events & SIGNAL_EVENT_CHILD)) _exit(8);

        event_shut();
        _exit(0);
    }

    int status = 0;

    TEST_ASSERT_EQ(waitpid(pid, &status, 0), pid);
    TEST_ASSERT(WIFEXITED(status));

    if (WIFEXITED(status)) {
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
}

void test_signal_reset_child_defaults(void)
{
    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0) {
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);

        sigset_t block;
        sigemptyset(&block);
        sigaddset(&block, SIGINT);
        sigaddset(&block, SIGCHLD);

        if (sigprocmask(SIG_BLOCK, &block, NULL) < 0) _exit(1);

        signal_reset_child();

        int signals[] = {
            SIGINT,
            SIGQUIT,
            SIGTSTP,
            SIGTTIN,
            SIGTTOU,
            SIGCHLD
        };

        for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); i++) {
            struct sigaction sa;

            if (sigaction(signals[i], NULL, &sa) < 0) _exit(2);
            if (sa.sa_handler != SIG_DFL) _exit(3);
        }

        sigset_t current;

        if (sigprocmask(SIG_SETMASK, NULL, &current) < 0) _exit(4);

        if (sigismember(&current, SIGINT)) _exit(5);
        if (sigismember(&current, SIGCHLD)) _exit(6);

        _exit(0);
    }

    int status = 0;

    TEST_ASSERT_EQ(waitpid(pid, &status, 0), pid);
    TEST_ASSERT(WIFEXITED(status));

    if (WIFEXITED(status)) {
        TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    }
}

void test_job_exit_status_last_process(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    Job *job = job_add(&manager, 1001, "pipeline");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) return;

    TEST_ASSERT_EQ(process_add(job, 2001), 0);
    TEST_ASSERT_EQ(process_add(job, 2002), 0);

    Process *first = job->processes;
    Process *last = first ? first->next : NULL;

    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_NOT_NULL(last);

    if (first && last) {
        first->status = PROCESS_DONE;
        first->wait_status = 0;
        first->wait_status_valid = 1;

        last->status = PROCESS_DONE;
        last->wait_status = 7 << 8;
        last->wait_status_valid = 1;

        TEST_ASSERT_EQ(job_exit_status(job), 7);
    }

    job_destroy(&manager);
}

void test_job_wait_foreground_pipeline_status(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    pid_t first = fork();

    TEST_ASSERT(first >= 0);

    if (first < 0) return;

    if (first == 0) {
        if (setpgid(0, 0) < 0) _exit(120);
        sleep_ms(200);
        _exit(0);
    }

    if (setpgid_parent(first, first) < 0) {
        kill(first, SIGKILL);
        waitpid(first, NULL, 0);
        return;
    }

    pid_t last = fork();

    TEST_ASSERT(last >= 0);

    if (last < 0) {
        kill(first, SIGKILL);
        waitpid(first, NULL, 0);
        return;
    }

    if (last == 0) {
        if (setpgid(0, first) < 0) _exit(121);
        _exit(7);
    }

    if (setpgid_parent(last, first) < 0) {
        kill(first, SIGKILL);
        kill(last, SIGKILL);
        waitpid(first, NULL, 0);
        waitpid(last, NULL, 0);
        return;
    }

    Job *job = job_add(&manager, first, "slow | false");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(-first, SIGKILL);
        waitpid(first, NULL, 0);
        waitpid(last, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, first), 0);
    TEST_ASSERT_EQ(process_add(job, last), 0);

    TEST_ASSERT_EQ(job_wait_foreground(job), 0);
    TEST_ASSERT_EQ(job->status, JOB_DONE);
    TEST_ASSERT_EQ(job_exit_status(job), 7);

    Process *process = job->processes;

    TEST_ASSERT_NOT_NULL(process);

    if (process) {
        TEST_ASSERT_EQ(process->status, PROCESS_DONE);

        process = process->next;

        TEST_ASSERT_NOT_NULL(process);

        if (process) {
            TEST_ASSERT_EQ(process->status, PROCESS_DONE);
        }
    }

    errno = 0;
    TEST_ASSERT_EQ(waitpid(-1, NULL, WNOHANG), -1);
    TEST_ASSERT_EQ(errno, ECHILD);

    job_destroy(&manager);
}

void test_job_wait_foreground_whole_group_stop(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    pid_t first = fork();

    TEST_ASSERT(first >= 0);

    if (first < 0) return;

    if (first == 0) {
        if (setpgid(0, 0) < 0) _exit(120);
        for (;;) pause();
    }

    if (setpgid_parent(first, first) < 0) {
        kill(first, SIGKILL);
        waitpid(first, NULL, 0);
        return;
    }

    pid_t second = fork();

    TEST_ASSERT(second >= 0);

    if (second < 0) {
        kill(first, SIGKILL);
        waitpid(first, NULL, 0);
        return;
    }

    if (second == 0) {
        if (setpgid(0, first) < 0) _exit(121);
        for (;;) pause();
    }

    if (setpgid_parent(second, first) < 0) {
        kill(first, SIGKILL);
        kill(second, SIGKILL);
        waitpid(first, NULL, 0);
        waitpid(second, NULL, 0);
        return;
    }

    Job *job = job_add(&manager, first, "pipeline_stop");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(-first, SIGKILL);
        waitpid(first, NULL, 0);
        waitpid(second, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, first), 0);
    TEST_ASSERT_EQ(process_add(job, second), 0);

    TEST_ASSERT_EQ(kill(-first, SIGSTOP), 0);

    TEST_ASSERT_EQ(job_wait_foreground(job), 0);
    TEST_ASSERT_EQ(job->status, JOB_STOPPED);

    Process *process = job->processes;

    while (process) {
        TEST_ASSERT_EQ(process->status, PROCESS_STOPPED);
        process = process->next;
    }

    job_shutdown(&manager);

    TEST_ASSERT_NULL(manager.head);

    errno = 0;
    TEST_ASSERT_EQ(waitpid(first, NULL, WNOHANG), -1);
    TEST_ASSERT_EQ(errno, ECHILD);

    errno = 0;
    TEST_ASSERT_EQ(waitpid(second, NULL, WNOHANG), -1);
    TEST_ASSERT_EQ(errno, ECHILD);
}


