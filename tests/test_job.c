#include "job.h"
#include "test_framework.h"
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_10ms(void)
{
    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = 10000000
    };

    nanosleep(&ts, NULL);
}

void test_job_init(void)
{
    Job job;
    job_init(&job);
    TEST_ASSERT_EQ(job.id, 0);
    TEST_ASSERT_EQ(job.pgid, 0);
    TEST_ASSERT_EQ(job.status, JOB_RUNNING);
    TEST_ASSERT_STR_EQ(job.command, "");
    TEST_ASSERT_NULL(job.processes);
    TEST_ASSERT_NULL(job.next);
}

void test_jobmanager_init(void)
{
    JobManager manager;
    jobmanager_init(&manager);
    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_EQ(manager.nextid, 1);
}

void test_job_add(void)
{
    JobManager manager;
    jobmanager_init(&manager);
    Job *job1 = job_add(&manager, 1001, "sleep 10");
    TEST_ASSERT_NOT_NULL(job1);

    if (!job1) return;

    TEST_ASSERT_EQ(job1->id, 1);
    TEST_ASSERT_EQ(job1->pgid, 1001);
    TEST_ASSERT_EQ(job1->status, JOB_RUNNING);
    TEST_ASSERT_STR_EQ(job1->command, "sleep 10");
    TEST_ASSERT_NULL(job1->processes);
    TEST_ASSERT_NULL(job1->next);

    Job *job2 = job_add(&manager, 1002, "ls");

    TEST_ASSERT_NOT_NULL(job2);

    if (!job2) {
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT_EQ(job2->id, 2);
    TEST_ASSERT_EQ(job2->pgid, 1002);
    TEST_ASSERT_EQ(job2->status, JOB_RUNNING);
    TEST_ASSERT_STR_EQ(job2->command, "ls");
    TEST_ASSERT_EQ(manager.nextid, 3);
    TEST_ASSERT(manager.head == job2);
    TEST_ASSERT(job2->next == job1);
    TEST_ASSERT_NULL(job1->next);

    job_destroy(&manager);
}

void test_job_find(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    Job *job1 = job_add(&manager, 1001, "job1");
    Job *job2 = job_add(&manager, 1002, "job2");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (!job1 || !job2) {
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT(job_find(&manager, 1001) == job1);
    TEST_ASSERT(job_find(&manager, 1002) == job2);
    TEST_ASSERT_NULL(job_find(&manager, 9999));

    job_destroy(&manager);
}

void test_process_add(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    Job *job = job_add(&manager, 3001, "test");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) return;

    TEST_ASSERT_EQ(process_add(job, 2001), 0);
    TEST_ASSERT_NOT_NULL(job->processes);

    if (job->processes) {
        TEST_ASSERT_EQ(job->processes->pid, 2001);
        TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);
        TEST_ASSERT_NULL(job->processes->next);
    }

    TEST_ASSERT_EQ(process_add(job, 2002), 0);
    TEST_ASSERT_NOT_NULL(job->processes->next);

    if (job->processes->next) {
        TEST_ASSERT_EQ(job->processes->next->pid, 2002);
        TEST_ASSERT_EQ(job->processes->next->status, PROCESS_RUNNING);
        TEST_ASSERT_NULL(job->processes->next->next);
    }

    job_destroy(&manager);
}

void test_job_count_active(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    TEST_ASSERT_EQ(job_count_active(&manager), 0);

    Job *job1 = job_add(&manager, 1001, "job1");
    Job *job2 = job_add(&manager, 1002, "job2");
    Job *job3 = job_add(&manager, 1003, "job3");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);
    TEST_ASSERT_NOT_NULL(job3);

    if (!job1 || !job2 || !job3) {
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT_EQ(job_count_active(&manager), 3);

    job1->status = JOB_DONE;
    TEST_ASSERT_EQ(job_count_active(&manager), 2);

    job2->status = JOB_STOPPED;
    TEST_ASSERT_EQ(job_count_active(&manager), 2);

    job3->status = JOB_DONE;
    TEST_ASSERT_EQ(job_count_active(&manager), 1);

    job2->status = JOB_DONE;
    TEST_ASSERT_EQ(job_count_active(&manager), 0);

    job_destroy(&manager);
}

void test_job_reap_signal(void)
{
    JobManager manager;
    int done = 0;

    jobmanager_init(&manager);

    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0) {
        for (;;) pause();
    }

    Job *job = job_add(&manager, pid, "test_signal");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(kill(pid, SIGTERM), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE) {
            done = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_reap_exit(void)
{
    JobManager manager;
    int done = 0;

    jobmanager_init(&manager);

    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;
    if (pid == 0) _exit(0);

    Job *job = job_add(&manager, pid, "test_exit");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE) {
            done = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_reap_exit_nonzero(void)
{
    JobManager manager;
    int done = 0;

    jobmanager_init(&manager);

    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;
    if (pid == 0) _exit(42);

    Job *job = job_add(&manager, pid, "test_exit_42");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE) {
            done = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_reap_stop_continue(void)
{
    JobManager manager;
    int stopped = 0;
    int continued = 0;
    int done = 0;

    jobmanager_init(&manager);

    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0) {
        for (;;) pause();
    }

    Job *job = job_add(&manager, pid, "test_stop_continue");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(kill(pid, SIGSTOP), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_STOPPED) {
            stopped = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(stopped, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_STOPPED);
    TEST_ASSERT_EQ(job->status, JOB_STOPPED);

    TEST_ASSERT_EQ(kill(pid, SIGCONT), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_RUNNING) {
            continued = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(continued, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);

    TEST_ASSERT_EQ(kill(pid, SIGTERM), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE) {
            done = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_remove(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    Job *job1 = job_add(&manager, 1001, "job1");
    Job *job2 = job_add(&manager, 1002, "job2");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (!job1 || !job2) {
        job_destroy(&manager);
        return;
    }

    job_remove(&manager, 1002);

    TEST_ASSERT_NULL(job_find(&manager, 1002));
    TEST_ASSERT(job_find(&manager, 1001) == job1);
    TEST_ASSERT(manager.head == job1);

    job_remove(&manager, 1001);

    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_NULL(job_find(&manager, 1001));
}

void test_job_destroy(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    Job *job1 = job_add(&manager, 1001, "job1");
    Job *job2 = job_add(&manager, 1002, "job2");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (job1) {
        TEST_ASSERT_EQ(process_add(job1, 2001), 0);
        TEST_ASSERT_EQ(process_add(job1, 2002), 0);
    }

    if (job2) TEST_ASSERT_EQ(process_add(job2, 3001), 0);

    job_destroy(&manager);

    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_EQ(manager.nextid, 1);
}

void test_job_cleanup_done(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    Job *running_job = job_add(&manager, 1001, "running");
    Job *done_job = job_add(&manager, 1002, "done");

    TEST_ASSERT_NOT_NULL(running_job);
    TEST_ASSERT_NOT_NULL(done_job);

    if (!running_job || !done_job) {
        job_destroy(&manager);
        return;
    }

    done_job->status = JOB_DONE;

    job_cleanup_done(&manager);

    TEST_ASSERT_NULL(job_find(&manager, 1002));
    TEST_ASSERT(job_find(&manager, 1001) == running_job);
    TEST_ASSERT(manager.head == running_job);

    job_destroy(&manager);
}

void test_job_continue(void)
{
    JobManager manager;
    int stopped = 0;
    int continued = 0;
    int done = 0;

    jobmanager_init(&manager);

    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0) {
        if (setpgid(0, 0) < 0) _exit(1);
        for (;;) pause();
    }

    if (setpgid(pid, pid) < 0 && errno != EACCES) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return;
    }

    Job *job = job_add(&manager, pid, "test_job_continue");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(job->pgid, pid);
    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(kill(pid, SIGSTOP), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_STOPPED) {
            stopped = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(stopped, 1);
    TEST_ASSERT_EQ(job->status, JOB_STOPPED);
    TEST_ASSERT_EQ(job_continue(job), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_RUNNING) {
            continued = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(continued, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);
    TEST_ASSERT_EQ(kill(pid, SIGTERM), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE) {
            done = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_multi_process(void)
{
    JobManager manager;
    int done1 = 0;
    int done2 = 0;

    jobmanager_init(&manager);

    pid_t pid1 = fork();

    TEST_ASSERT(pid1 >= 0);

    if (pid1 < 0) return;

    if (pid1 == 0) {
        for (;;) pause();
    }

    pid_t pid2 = fork();

    TEST_ASSERT(pid2 >= 0);

    if (pid2 < 0) {
        kill(pid1, SIGTERM);
        waitpid(pid1, NULL, 0);
        return;
    }

    if (pid2 == 0) {
        for (;;) pause();
    }

    Job *job = job_add(&manager, pid1, "test_multi_process");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(pid1, SIGTERM);
        kill(pid2, SIGTERM);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid1), 0);
    TEST_ASSERT_EQ(process_add(job, pid2), 0);

    if (!job->processes) {
        TEST_ASSERT(0);
        kill(pid1, SIGTERM);
        kill(pid2, SIGTERM);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT_NOT_NULL(job->processes);

    if (!job->processes->next) {
        TEST_ASSERT(0);
        kill(pid1, SIGTERM);
        kill(pid2, SIGTERM);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT_NOT_NULL(job->processes->next);

    TEST_ASSERT_EQ(job->processes->pid, pid1);
    TEST_ASSERT_EQ(job->processes->next->pid, pid2);

    TEST_ASSERT_EQ(kill(pid1, SIGTERM), 0);

    for (int i = 0; i < 100; i++) {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE) {
            done1 = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done1, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->processes->next->status, PROCESS_RUNNING);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);

    TEST_ASSERT_EQ(kill(pid2, SIGTERM), 0);

    for (int i = 0; i < 100; i++)
    {
        job_reap(&manager);

        if (job->processes->next->status == PROCESS_DONE)
        {
            done2 = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(done2, 1);
    TEST_ASSERT_EQ(job->processes->next->status, PROCESS_DONE);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_mixed_done_stopped(void)
{
    JobManager manager;
    int ready = 0;
    int cleaned = 0;

    jobmanager_init(&manager);

    pid_t done_pid = fork();

    TEST_ASSERT(done_pid >= 0);

    if (done_pid < 0) return;
    if (done_pid == 0) _exit(0);

    pid_t stopped_pid = fork();

    TEST_ASSERT(stopped_pid >= 0);

    if (stopped_pid < 0) {
        waitpid(done_pid, NULL, 0);
        return;
    }

    if (stopped_pid == 0) {
        for (;;) pause();
    }

    Job *job = job_add(&manager, done_pid, "mixed_done_stopped");

    TEST_ASSERT_NOT_NULL(job);

    if (!job) {
        kill(stopped_pid, SIGTERM);
        waitpid(done_pid, NULL, 0);
        waitpid(stopped_pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, done_pid), 0);
    TEST_ASSERT_EQ(process_add(job, stopped_pid), 0);
    TEST_ASSERT_EQ(kill(stopped_pid, SIGSTOP), 0);

    for (int i = 0; i < 100; i++)
    {
        job_reap(&manager);

        Process *first = job->processes;
        Process *second = first ? first->next : NULL;

        if (first &&second &&first->status == PROCESS_DONE &&second->status == PROCESS_STOPPED)
        {
            ready = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(ready, 1);

    if (ready) {
        TEST_ASSERT_EQ(job->processes->status, PROCESS_DONE);
        TEST_ASSERT_EQ(job->processes->next->status, PROCESS_STOPPED);
        TEST_ASSERT_EQ(job->status, JOB_STOPPED);
    }

    kill(stopped_pid, SIGCONT);
    kill(stopped_pid, SIGTERM);

    for (int i = 0; i < 100; i++)
    {
        job_reap(&manager);

        if (job->status == JOB_DONE)
        {
            cleaned = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(cleaned, 1);
    TEST_ASSERT_EQ(job->status, JOB_DONE);

    job_destroy(&manager);
}

void test_job_shutdown_running_process(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    pid_t pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0) return;

    if (pid == 0)
    {
        if (setpgid(0, 0) < 0)
            _exit(1);
        for (;;)
            pause();
    }

    if (setpgid(pid, pid) < 0 && errno != EACCES)
    {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return;
    }

    Job *job = job_add(&manager, pid, "shutdown_running");

    TEST_ASSERT_NOT_NULL(job);

    if (!job)
    {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    job_shutdown(&manager);
    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_EQ(manager.nextid, 1);
    errno = 0;
    pid_t ret = waitpid(pid, NULL, WNOHANG);
    TEST_ASSERT_EQ(ret, -1);
    TEST_ASSERT_EQ(errno, ECHILD);
}

void test_job_shutdown_stopped_process(void)
{
    JobManager manager;
    jobmanager_init(&manager);
    pid_t pid = fork();
    TEST_ASSERT(pid >= 0);
    if (pid < 0)
        return;

    if (pid == 0)
    {
        if (setpgid(0, 0) < 0)
            _exit(1);
        for (;;)
            pause();
    }

    if (setpgid(pid, pid) < 0 && errno != EACCES)
    {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return;
    }

    Job *job = job_add(&manager, pid, "shutdown_stopped");

    TEST_ASSERT_NOT_NULL(job);

    if (!job)
    {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(kill(pid, SIGSTOP), 0);

    int stopped = 0;

    for (int i = 0; i < 100; i++)
    {
        job_reap(&manager);
        if (job->status == JOB_STOPPED)
        {
            stopped = 1;
            break;
        }
        sleep_10ms();
    }

    TEST_ASSERT_EQ(stopped, 1);
    TEST_ASSERT_EQ(job->status, JOB_STOPPED);
    job_shutdown(&manager);
    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_EQ(manager.nextid, 1);
    errno = 0;
    pid_t ret = waitpid(pid, NULL, WNOHANG);
    TEST_ASSERT_EQ(ret, -1);
    TEST_ASSERT_EQ(errno, ECHILD);
}

void test_job_shutdown_multiple_processes(void)
{
    JobManager manager;
    jobmanager_init(&manager);

    pid_t pid1 = fork();
    TEST_ASSERT(pid1 >= 0);

    if (pid1 < 0)
        return;

    if (pid1 == 0)
    {
        if (setpgid(0, 0) < 0)
            _exit(1);
        for (;;)
            pause();
    }

    if (setpgid(pid1, pid1) < 0 && errno != EACCES)
    {
        kill(pid1, SIGKILL);
        waitpid(pid1, NULL, 0);
        return;
    }

    pid_t pid2 = fork();
    TEST_ASSERT(pid2 >= 0);

    if (pid2 < 0)
    {
        kill(pid1, SIGKILL);
        waitpid(pid1, NULL, 0);
        return;
    }

    if (pid2 == 0)
    {
        if (setpgid(0, 0) < 0)
            _exit(1);
        for (;;)
            pause();
    }

    if (setpgid(pid2, pid2) < 0 && errno != EACCES)
    {
        kill(pid1, SIGKILL);
        kill(pid2, SIGKILL);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return;
    }

    Job *job1 = job_add(&manager, pid1, "shutdown_one");
    Job *job2 = job_add(&manager, pid2, "shutdown_two");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (!job1 || !job2)
    {
        kill(pid1, SIGKILL);
        kill(pid2, SIGKILL);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT_EQ(process_add(job1, pid1), 0);
    TEST_ASSERT_EQ(process_add(job2, pid2), 0);
    TEST_ASSERT_EQ(job_count_active(&manager), 2);
    job_shutdown(&manager);
    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_EQ(job_count_active(&manager), 0);

    errno = 0;
    TEST_ASSERT_EQ(waitpid(pid1, NULL, WNOHANG), -1);
    TEST_ASSERT_EQ(errno, ECHILD);

    errno = 0;
    TEST_ASSERT_EQ(waitpid(pid2, NULL, WNOHANG), -1);
    TEST_ASSERT_EQ(errno, ECHILD);
}

