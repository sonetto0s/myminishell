#include "executor.h"
#include "terminal.h"
#include "sig.h"
#include "job.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void run_process(Command *com);

static int setpgid_child(pid_t pgid)
{
    while (setpgid(0, pgid) < 0)
    {
        if (errno == EINTR)
            continue;
        return -1;
    }

    return 0;
}

static int setpgid_parent(pid_t pid, pid_t pgid)
{
    while (setpgid(pid, pgid) < 0)
    {
        if (errno == EINTR)
            continue;
        if (errno == EACCES || errno == ESRCH)
            return 0;

        return -1;
    }

    return 0;
}

static void terminate_and_reap_child(pid_t pid)
{
    if (pid <= 0)
        return;

    if (kill(pid, SIGTERM) < 0 && errno != ESRCH)
    {
        log_error("failed terminate child %d", pid);
    }

    while (1)
    {
        pid_t ret = waitpid(pid, NULL, 0);

        if (ret == pid) return;
        if (ret < 0 &&errno == EINTR)
            continue;
        if (ret < 0 &&errno == ECHILD)
            return;

        if (ret < 0)
            log_error("failed reap child %d", pid);
        return;
    }
}

static void terminate_and_reap_pipeline(pid_t *pids, int count)
{
    if (!pids || count <= 0)
        return;

    for (int i = 0; i < count; i++)
    {
        if (pids[i] <= 0) continue;

        if (kill(pids[i], SIGKILL) < 0 && errno != ESRCH)
        {
            log_error("failed terminate pipeline child %d", pids[i]);
        }
    }

    for (int i = 0; i < count; i++)
    {
        if (pids[i] <= 0)
            continue;

        while (1)
        {
            pid_t ret = waitpid(pids[i], NULL, 0);

            if (ret == pids[i])
                break;
            if (ret < 0 &&errno == EINTR)
                continue;
            if (ret < 0 &&errno == ECHILD)
                break;

            if (ret < 0)
                log_error("failed reap pipeline child %d", pids[i]);
            break;
        }
    }
}

static int check_job_limit(ShellContext *ctx)
{
    if (!ctx) return MiniShell_ERR_UNKNOWN;

    if (job_count_active(&ctx->jobs) >= ctx->config.max_job) {
        log_error("maximum active jobs reached: %d", ctx->config.max_job);
        return MiniShell_ERR_JOB;
    }

    return MiniShell_OK;
}

static int finish_foreground_job(JobManager *manager, Job *job, int terminal_active)
{
    if (!manager || !job) return MiniShell_ERR_UNKNOWN;

    if (terminal_active && terminal_restore() < 0) {
        log_error("failed restore terminal");
    }

    if (job->status == JOB_STOPPED) {
        printf("\n[%d]+ Stopped %s\n", job->id, job->command);
        return MiniShell_OK;
    }

    if (job->status != JOB_DONE) return MiniShell_ERR_UNKNOWN;

    int status = job_exit_status(job);
    pid_t pgid = job->pgid;

    job_remove(manager, pgid);

    if (status < 0) return MiniShell_ERR_UNKNOWN;

    return status;
}

int execute_command(Command *com, ShellContext *ctx)
{
    if (!com || !ctx) return MiniShell_ERR_UNKNOWN;

    int ret = check_job_limit(ctx);
    if (ret != MiniShell_OK) return ret;

    if (!com->next) return execute_single(com, ctx);

    return execute_pipeline(com, ctx);
}

int setredirect(Command *com)
{
    if (!com) return MiniShell_ERR_UNKNOWN;

    if (com->redirect.output_file) {
        int flags = com->redirect.append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
        int fd = open(com->redirect.output_file, flags, 0644);

        if (fd < 0) {
            log_error("open file failed: %s", com->redirect.output_file);
            return MiniShell_ERR_OPEN;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            close(fd);
            log_error("dup2 failed");
            return MiniShell_ERR_DUP2;
        }

        close(fd);
    }

    if (com->redirect.input_file) {
        int fd = open(com->redirect.input_file, O_RDONLY);

        if (fd < 0) {
            log_error("open file failed: %s", com->redirect.input_file);
            return MiniShell_ERR_OPEN;
        }

        if (dup2(fd, STDIN_FILENO) < 0) {
            close(fd);
            log_error("dup2 failed");
            return MiniShell_ERR_DUP2;
        }

        close(fd);
    }

    return MiniShell_OK;
}

int execute_single(Command *com, ShellContext *ctx)
{
    if (!com || !ctx) return MiniShell_ERR_UNKNOWN;

    int terminal_active = terminal_is_initialized();
    pid_t pid = fork();

    if (pid < 0) {
        log_error("fork failed");
        return MiniShell_ERR_FORK;
    }

    if (pid == 0) {
        if (setpgid_child(0) < 0) _exit(1);

        if (setredirect(com) != MiniShell_OK) _exit(1);

        run_process(com);
    }

    if (setpgid_parent(pid, pid) < 0) {
        log_error("failed setpgid parent");
        terminate_and_reap_child(pid);
        return MiniShell_ERR_UNKNOWN;
    }

    Job *job = job_add(&ctx->jobs, pid, com->argv[0]);

    if (!job) {
        terminate_and_reap_child(pid);
        return MiniShell_ERR_JOB;
    }

    if (process_add(job, pid) < 0) {
        terminate_and_reap_child(pid);
        job_remove(&ctx->jobs, pid);
        return MiniShell_ERR_JOB;
    }

    if (com->background) return MiniShell_OK;

    if (terminal_active && terminal_set_foreground(pid) < 0) {
        log_error("failed give terminal foreground");
        terminate_and_reap_child(pid);
        job_remove(&ctx->jobs, pid);
        terminal_restore();
        return MiniShell_ERR_UNKNOWN;
    }

    if (job_wait_foreground(job) < 0) {
        terminate_and_reap_child(pid);

        if (terminal_active) terminal_restore();

        job_remove(&ctx->jobs, pid);
        return MiniShell_ERR_UNKNOWN;
    }

    return finish_foreground_job(&ctx->jobs, job, terminal_active);
}

int execute_pipeline(Command *com, ShellContext *ctx)
{
    if (!com || !ctx) return MiniShell_ERR_UNKNOWN;

    Command *current = com;
    int pipe_fd = -1;
    int count = 0;
    int terminal_active = terminal_is_initialized();
    pid_t pids[MAX_PIPELINE];

    for (int i = 0; i < MAX_PIPELINE; i++) pids[i] = -1;

    pid_t pgid = 0;

    while (current) {
        int pipefd[2] = {-1, -1};

        if (count >= MAX_PIPELINE) {
            if (pipe_fd >= 0) close(pipe_fd);

            terminate_and_reap_pipeline(pids, count);
            log_error("pipeline commands are too many");
            return MiniShell_ERR_UNKNOWN;
        }

        if (current->next && pipe(pipefd) < 0) {
            if (pipe_fd >= 0) close(pipe_fd);

            terminate_and_reap_pipeline(pids, count);
            log_error("pipe create failed");
            return MiniShell_ERR_PIPE;
        }

        pid_t pid = fork();

        if (pid < 0) {
            if (pipefd[0] >= 0) close(pipefd[0]);
            if (pipefd[1] >= 0) close(pipefd[1]);
            if (pipe_fd >= 0) close(pipe_fd);

            terminate_and_reap_pipeline(pids, count);
            log_error("fork failed");
            return MiniShell_ERR_FORK;
        }

        if (pid == 0) {
            pid_t child_pgid = pgid == 0 ? getpid() : pgid;

            if (setpgid_child(child_pgid) < 0) _exit(1);

            if (pipe_fd >= 0 && dup2(pipe_fd, STDIN_FILENO) < 0) _exit(127);
            if (current->next && dup2(pipefd[1], STDOUT_FILENO) < 0) _exit(127);

            if (pipe_fd >= 0) close(pipe_fd);
            if (pipefd[0] >= 0) close(pipefd[0]);
            if (pipefd[1] >= 0) close(pipefd[1]);

            if (setredirect(current) != MiniShell_OK) _exit(127);

            run_process(current);
        }

        pids[count++] = pid;

        if (pgid == 0) pgid = pid;

        if (setpgid_parent(pid, pgid) < 0) {
            if (pipefd[0] >= 0) close(pipefd[0]);
            if (pipefd[1] >= 0) close(pipefd[1]);
            if (pipe_fd >= 0) close(pipe_fd);

            terminate_and_reap_pipeline(pids, count);
            log_error("failed parent setpgid");
            return MiniShell_ERR_UNKNOWN;
        }

        if (pipe_fd >= 0) close(pipe_fd);
        pipe_fd = -1;

        if (current->next) {
            close(pipefd[1]);
            pipe_fd = pipefd[0];
        }

        current = current->next;
    }

    if (pipe_fd >= 0) close(pipe_fd);

    Job *job = job_add(&ctx->jobs, pgid, com->argv[0]);

    if (!job) {
        terminate_and_reap_pipeline(pids, count);
        return MiniShell_ERR_JOB;
    }

    for (int i = 0; i < count; i++) {
        if (process_add(job, pids[i]) < 0) {
            terminate_and_reap_pipeline(pids, count);
            job_remove(&ctx->jobs, pgid);
            return MiniShell_ERR_JOB;
        }
    }

    if (com->background) return MiniShell_OK;

    if (terminal_active && terminal_set_foreground(pgid) < 0) {
        terminate_and_reap_pipeline(pids, count);
        job_remove(&ctx->jobs, pgid);
        terminal_restore();
        log_error("failed set pipeline foreground");
        return MiniShell_ERR_UNKNOWN;
    }

    if (job_wait_foreground(job) < 0) {
        terminate_and_reap_pipeline(pids, count);

        if (terminal_active) terminal_restore();

        job_remove(&ctx->jobs, pgid);
        return MiniShell_ERR_UNKNOWN;
    }

    return finish_foreground_job(&ctx->jobs, job, terminal_active);
}

static void run_process(Command *com)
{
    event_close_in_child();
    signal_reset_child();

    execvp(com->argv[0], com->argv);

    log_error("execute command failed: %s", com->argv[0]);
    _exit(127);
}


