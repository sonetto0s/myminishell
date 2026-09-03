#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "job.h"
#include "test_framework.h"

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
    Job *job1;
    Job *job2;

    jobmanager_init(&manager);

    job1 = job_add(&manager, 1001, "sleep 10");

    TEST_ASSERT_NOT_NULL(job1);

    if (job1 == NULL)
        return;

    TEST_ASSERT_EQ(job1->id, 1);
    TEST_ASSERT_EQ(job1->pgid, 1001);
    TEST_ASSERT_EQ(job1->status, JOB_RUNNING);
    TEST_ASSERT_STR_EQ(job1->command, "sleep 10");
    TEST_ASSERT_NULL(job1->processes);
    TEST_ASSERT_NULL(job1->next);

    job2 = job_add(&manager, 1002, "ls");

    TEST_ASSERT_NOT_NULL(job2);

    if (job2 == NULL)
    {
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
    Job *job1;
    Job *job2;

    jobmanager_init(&manager);

    job1 = job_add(&manager, 1001, "job1");
    job2 = job_add(&manager, 1002, "job2");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (job1 == NULL || job2 == NULL)
    {
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
    Job *job;

    jobmanager_init(&manager);

    job = job_add(&manager, 3001, "test");

    TEST_ASSERT_NOT_NULL(job);

    if (job == NULL)
        return;

    TEST_ASSERT_EQ(process_add(job, 2001), 0);

    TEST_ASSERT_NOT_NULL(job->processes);

    if (job->processes != NULL)
    {
        TEST_ASSERT_EQ(job->processes->pid, 2001);
        TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);
        TEST_ASSERT_NULL(job->processes->next);
    }

    TEST_ASSERT_EQ(process_add(job, 2002), 0);

    TEST_ASSERT_NOT_NULL(job->processes->next);

    if (job->processes->next != NULL)
    {
        TEST_ASSERT_EQ(job->processes->next->pid, 2002);
        TEST_ASSERT_EQ(
            job->processes->next->status,
            PROCESS_RUNNING
        );
        TEST_ASSERT_NULL(job->processes->next->next);
    }

    job_destroy(&manager);
}

void test_job_reap_signal(void)
{
    JobManager manager;
    Job *job;
    pid_t pid;
    int done = 0;

    jobmanager_init(&manager);

    pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0)
        return;

    if (pid == 0)
    {
        for (;;)
            pause();
    }

    job = job_add(&manager, pid, "test_signal");

    TEST_ASSERT_NOT_NULL(job);

    if (job == NULL)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);

    TEST_ASSERT_EQ(kill(pid, SIGTERM), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE)
        {
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
    Job *job;
    pid_t pid;
    int done = 0;

    jobmanager_init(&manager);

    pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0)
        return;

    if (pid == 0)
    {
        _exit(0);
    }

    job = job_add(&manager, pid, "test_exit");

    TEST_ASSERT_NOT_NULL(job);

    if (job == NULL)
    {
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE)
        {
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
    Job *job;
    pid_t pid;
    int done = 0;

    jobmanager_init(&manager);

    pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0)
        return;

    if (pid == 0)
    {
        _exit(42);
    }

    job = job_add(&manager, pid, "test_exit_42");

    TEST_ASSERT_NOT_NULL(job);

    if (job == NULL)
    {
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE)
        {
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
    Job *job;
    pid_t pid;
    int stopped = 0;
    int continued = 0;
    int done = 0;

    jobmanager_init(&manager);

    pid = fork();

    TEST_ASSERT(pid >= 0);

    if (pid < 0)
        return;

    if (pid == 0)
    {
        for (;;)
            pause();
    }

    job = job_add(&manager, pid, "test_stop_continue");

    TEST_ASSERT_NOT_NULL(job);

    if (job == NULL)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);

    /*
     * RUNNING -> STOPPED
     */
    TEST_ASSERT_EQ(kill(pid, SIGSTOP), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);

        if (job->processes->status == PROCESS_STOPPED)
        {
            stopped = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(stopped, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_STOPPED);
    TEST_ASSERT_EQ(job->status, JOB_STOPPED);

    /*
     * STOPPED -> RUNNING
     */
    TEST_ASSERT_EQ(kill(pid, SIGCONT), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);

        if (job->processes->status == PROCESS_RUNNING)
        {
            continued = 1;
            break;
        }

        sleep_10ms();
    }

    TEST_ASSERT_EQ(continued, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);

    /*
     * Cleanup child.
     */
    TEST_ASSERT_EQ(kill(pid, SIGTERM), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);

        if (job->processes->status == PROCESS_DONE)
        {
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
    Job *job1;
    Job *job2;

    jobmanager_init(&manager);

    job1 = job_add(&manager, 1001, "job1");
    job2 = job_add(&manager, 1002, "job2");

    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (job1 == NULL || job2 == NULL)
    {
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
    Job *job1;
    Job *job2;

    jobmanager_init(&manager);
    job1 = job_add(&manager, 1001, "job1");
    job2 = job_add(&manager, 1002, "job2");
    TEST_ASSERT_NOT_NULL(job1);
    TEST_ASSERT_NOT_NULL(job2);

    if (job1 != NULL)
        TEST_ASSERT_EQ(process_add(job1, 2001), 0);

    if (job1 != NULL)
        TEST_ASSERT_EQ(process_add(job1, 2002), 0);

    if (job2 != NULL)
        TEST_ASSERT_EQ(process_add(job2, 3001), 0);

    job_destroy(&manager);
    TEST_ASSERT_NULL(manager.head);
    TEST_ASSERT_EQ(manager.nextid, 1);
}

void test_job_cleanup_done(void)
{
    JobManager manager;
    Job *running_job;
    Job *done_job;

    jobmanager_init(&manager);
    running_job = job_add(&manager, 1001, "running");
    done_job = job_add(&manager, 1002, "done");
    TEST_ASSERT_NOT_NULL(running_job);
    TEST_ASSERT_NOT_NULL(done_job);

    if (running_job == NULL || done_job == NULL)
    {
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
    Job *job;
    pid_t pid;
    int stopped = 0;
    int continued = 0;
    int done = 0;
    jobmanager_init(&manager);
    pid = fork();
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

    if (setpgid(pid, pid) < 0)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return;
    }
    job = job_add(&manager, pid, "test_job_continue");
    TEST_ASSERT_NOT_NULL(job);
    if (job == NULL)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return;
    }
    TEST_ASSERT_EQ(job->pgid, pid);
    TEST_ASSERT_EQ(process_add(job, pid), 0);
    TEST_ASSERT_EQ(kill(pid, SIGSTOP), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);
        if (job->processes->status == PROCESS_STOPPED)
        {
            stopped = 1;
            break;
        }
        sleep_10ms();
    }

    TEST_ASSERT_EQ(stopped, 1);
    TEST_ASSERT_EQ(job->status, JOB_STOPPED);
    TEST_ASSERT_EQ(job_continue(job), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);
        if (job->processes->status == PROCESS_RUNNING)
        {
            continued = 1;
            break;
        }
        sleep_10ms();
    }
    TEST_ASSERT_EQ(continued, 1);
    TEST_ASSERT_EQ(job->processes->status, PROCESS_RUNNING);
    TEST_ASSERT_EQ(job->status, JOB_RUNNING);

    TEST_ASSERT_EQ(kill(pid, SIGTERM), 0);
    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);
        if (job->processes->status == PROCESS_DONE)
        {
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
    Job *job;
    pid_t pid1;
    pid_t pid2;
    int done1 = 0;
    int done2 = 0;
    jobmanager_init(&manager);
    pid1 = fork();
    TEST_ASSERT(pid1 >= 0);

    if (pid1 < 0)
        return;

    if (pid1 == 0)
    {
        for (;;)
            pause();
    }

    pid2 = fork();
    TEST_ASSERT(pid2 >= 0);
    if (pid2 < 0)
    {
        kill(pid1, SIGTERM);
        waitpid(pid1, NULL, 0);
        return;
    }

    if (pid2 == 0)
    {
        for (;;)
            pause();
    }

    job = job_add(&manager, pid1, "test_multi_process");
    TEST_ASSERT_NOT_NULL(job);
    if (job == NULL)
    {
        kill(pid1, SIGTERM);
        kill(pid2, SIGTERM);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid1), 0);
    TEST_ASSERT_EQ(process_add(job, pid2), 0);
    TEST_ASSERT_NOT_NULL(job->processes);
    TEST_ASSERT_NOT_NULL(job->processes->next);

    if (job->processes == NULL ||
        job->processes->next == NULL)
    {
        kill(pid1, SIGTERM);
        kill(pid2, SIGTERM);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        job_destroy(&manager);
        return;
    }

    TEST_ASSERT_EQ(job->processes->pid, pid1);
    TEST_ASSERT_EQ(job->processes->next->pid, pid2);
    TEST_ASSERT_EQ(kill(pid1, SIGTERM), 0);

    for (int i = 0; i < 100; ++i)
    {
        job_reap(&manager);
        if (job->processes->status == PROCESS_DONE)
        {
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

    for (int i = 0; i < 100; ++i)
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
