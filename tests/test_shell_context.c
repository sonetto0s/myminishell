#include "shell_context.h"
#include "job.h"
#include "test_framework.h"
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void test_shell_context_init(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    TEST_ASSERT_EQ(ctx.running, 1);
    TEST_ASSERT_EQ(ctx.last_exit_status, 0);
    TEST_ASSERT_NULL(ctx.jobs.head);
    TEST_ASSERT_EQ(ctx.jobs.nextid, 1);
    TEST_ASSERT_STR_EQ(ctx.config_file, "config/config.conf");
    shell_context_destroy(&ctx);
}

void test_shell_context_destroy(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    TEST_ASSERT_EQ(ctx.running, 1);
    shell_context_destroy(&ctx);
    TEST_ASSERT_EQ(ctx.running, 0);
    TEST_ASSERT_NULL(ctx.jobs.head);
}

void test_shell_context_destroy_reaps_jobs(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    pid_t pid = fork();
    TEST_ASSERT(pid >= 0);

    if (pid < 0)
    {
        shell_context_destroy(&ctx);
        return;
    }

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
        shell_context_destroy(&ctx);
        return;
    }

    Job *job = job_add(&ctx.jobs, pid, "context_shutdown_test");
    TEST_ASSERT_NOT_NULL(job);

    if (!job)
    {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        shell_context_destroy(&ctx);
        return;
    }

    TEST_ASSERT_EQ(process_add(job, pid), 0);
    shell_context_destroy(&ctx);
    TEST_ASSERT_EQ(ctx.running, 0);
    TEST_ASSERT_NULL(ctx.jobs.head);
    errno = 0;
    pid_t ret = waitpid(pid, NULL, WNOHANG);
    TEST_ASSERT_EQ(ret, -1);
    TEST_ASSERT_EQ(errno, ECHILD);
}


