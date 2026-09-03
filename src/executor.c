#include "executor.h"
#include "terminal.h"
#include "sig.h"
#include <signal.h>
#include "job.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "error.h"
#include "log.h"
#include "event.h"


static void run_process(Command *com);

static void cleanup_pipeline(pid_t pgid)
{
    if (pgid <= 0)
        return;
    kill(-pgid, SIGTERM);
    while (1)
    {
        pid_t ret = waitpid(-pgid, NULL, 0);
        if (ret > 0)
            continue;
        if (ret < 0 && errno == EINTR)
            continue;
        break;
    }
}
int execute_command(Command *com, ShellContext *ctx)
{
    if(!com->next)
        return execute_single(com,ctx);
    else
        return execute_pipeline(com,ctx);
}

int setredirect(Command *com)
{
    if (com->redirect.output_file)
    {
        int flags;
        if(com->redirect.append)
        {
            flags = O_WRONLY | O_CREAT | O_APPEND;
        }
        else
       {
          flags = O_WRONLY | O_CREAT | O_TRUNC;
       }
          int fd = open(com->redirect.output_file, flags, 0644);
          if (fd < 0)
          {
              log_error("open file failed: %s ", com->redirect.output_file);
              return MiniShell_ERR_OPEN;
          }
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            close(fd);
            log_error("dup2 failed");
            return MiniShell_ERR_DUP2;
        }
        close(fd);
    }

    if (com->redirect.input_file)
    {
        int fd = open(com->redirect.input_file, O_RDONLY, 0644);
        if (fd < 0)
        {
            log_error("open file failed: %s ", com->redirect.input_file);
            return MiniShell_ERR_OPEN;
        }
        if (dup2(fd, STDIN_FILENO) == -1)
        {
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
    int status;
    int terminal_active = terminal_is_initialized();
    pid_t pid = fork();
    if (pid < 0)
    {
        log_error("fork failed");
        return MiniShell_ERR_FORK;
    }
    if (pid == 0)
    {
        if (setpgid(0, 0) < 0)
        {
            log_error("failed setpgid child");
            _exit(1);
        }
        if (setredirect(com) != MiniShell_OK)
        {
            log_error("redirect failed");
            _exit(1);
        }
        run_process(com);
    }


    if (setpgid(pid, pid) < 0)
    {
        if (errno != EACCES)
        {
            log_error("failed setpgid parent");
            return MiniShell_ERR_UNKNOWN;
        }
    }

    Job *job = job_add(&ctx->jobs, pid, com->argv[0]);

    if (job == NULL)
    {
        log_error("failed add job");
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        return MiniShell_ERR_UNKNOWN;
    }

    if (process_add(job, pid) < 0)
    {
        log_error("failed add process");
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
        job_remove(&ctx->jobs, pid);
        return MiniShell_ERR_UNKNOWN;
    }

    if (com->background)
    {
        return MiniShell_OK;
    }

    if (terminal_active)
    {
        if (terminal_set_foreground(pid) < 0)
        {
            log_error("failed give terminal foreground");
        }
    }

    while (1)
    {
        if (waitpid(pid, &status, WUNTRACED) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            log_error("failed wait");

            if (terminal_active)
                terminal_restore();

            job_remove(&ctx->jobs, pid);
            return MiniShell_ERR_UNKNOWN;
        }

        break;
    }

    if (WIFSTOPPED(status))
    {
        job->status = JOB_STOPPED;
        if (job->processes)
        {
            job->processes->status = PROCESS_STOPPED;
        }

        terminal_restore();
        printf("\n[%d]+ Stopped %s\n", job->id, job->command);
        return MiniShell_OK;
    }
    if (terminal_active)
        terminal_restore();

    job_remove(&ctx->jobs, pid);
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }

    return MiniShell_ERR_UNKNOWN;
}

int execute_pipeline(Command *com, ShellContext *ctx)
{
    Command *current = com;
    int pipe_fd = -1;
    int count = 0;
    int terminal_active = terminal_is_initialized();
    pid_t pids[MAX_PIPELINE];
    pid_t pgid = 0;
    int status = 0;
    int last_status = 0;
    while (current)
    {
        int pipefd[2];
        if (count >= MAX_PIPELINE)
        {
            log_error("pipeline commands are too many");
            cleanup_pipeline(pgid);
            return MiniShell_ERR_UNKNOWN;
        }
        if (current->next)
        {
            if (pipe(pipefd) < 0)
            {
                log_error("pipe create failed");

                if (pipe_fd != -1)
                    close(pipe_fd);

                cleanup_pipeline(pgid);
                return MiniShell_ERR_PIPE;
            }
        }
        pid_t pid = fork();

        if (pid < 0)
        {
            log_error("fork failed");

            if (current->next)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            if (pipe_fd != -1)
                close(pipe_fd);

            cleanup_pipeline(pgid);
            return MiniShell_ERR_FORK;
        }

        if (pid > 0)
        {
            if (pgid == 0)
                pgid = pid;

            if (setpgid(pid, pgid) < 0 && errno != EACCES)
                log_error("failed parent setpgid");

            pids[count++] = pid;
        }
        else
        {
            pid_t child_pgid = (pgid == 0) ? getpid() : pgid;

            if (setpgid(0, child_pgid) < 0)
            {
                log_error("failed child setpgid");
                _exit(1);
            }

            if (pipe_fd != -1)
            {
                if (dup2(pipe_fd, STDIN_FILENO) < 0)
                {
                    log_error("dup2 stdin failed");
                    _exit(127);
                }
            }

            if (current->next)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0)
                {
                    log_error("dup2 stdout failed");
                    _exit(127);
                }
            }

            if (pipe_fd != -1)
                close(pipe_fd);

            if (current->next)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            if (setredirect(current) != MiniShell_OK)
            {
                log_error("redirect failed");
                _exit(127);
            }

            run_process(current);
        }

        if (current->next)
        {
            if (pipe_fd != -1)
                close(pipe_fd);

            pipe_fd = pipefd[0];
            close(pipefd[1]);
        }

        current = current->next;
    }

    if (pipe_fd != -1)
        close(pipe_fd);

    Job *job = job_add(&ctx->jobs, pgid, com->argv[0]);

    if (job == NULL)
    {
        cleanup_pipeline(pgid);
        return MiniShell_ERR_UNKNOWN;
    }

    for (int i = 0; i < count; i++)
    {
        if (process_add(job, pids[i]) < 0)
        {
            log_error("failed add process");
            job_remove(&ctx->jobs, pgid);
            cleanup_pipeline(pgid);
            return MiniShell_ERR_UNKNOWN;
        }
    }

    if (com->background)
        return MiniShell_OK;

    if (terminal_active)
    {
        if (terminal_set_foreground(pgid) < 0)
            log_error("failed set pipeline foreground");
    }

    while (1)
    {
        pid_t ret = waitpid(-pgid, &status, WUNTRACED);

        if (ret < 0)
        {
            if (errno == EINTR)
                continue;

            log_error("waitpid pipeline failed");
            terminal_restore();
            job_remove(&ctx->jobs, pgid);
            return MiniShell_ERR_UNKNOWN;
        }

        Process *process = job->processes;

        while (process && process->pid != ret)
            process = process->next;

        if (process == NULL)
        {
            log_error("process not found in job");
            terminal_restore();
            job_remove(&ctx->jobs, pgid);
            return MiniShell_ERR_UNKNOWN;
        }

        if (WIFSTOPPED(status))
        {
            process->status = PROCESS_STOPPED;
        }
        else if (WIFEXITED(status) || WIFSIGNALED(status))
        {
            process->status = PROCESS_DONE;
        }

        if (ret == pids[count - 1])
            last_status = status;

        int running = 0;

        process = job->processes;

        while (process)
        {
            if (process->status == PROCESS_RUNNING)
            {
                running = 1;
                break;
            }

            process = process->next;
        }

        if (!running)
            break;
    }

    if (terminal_restore() < 0)
        log_error("failed restore terminal");

    Process *process = job->processes;
    int all_done = 1;

    while (process)
    {
        if (process->status != PROCESS_DONE)
        {
            all_done = 0;
            break;
        }

        process = process->next;
    }

    if (all_done)
    {
        job->status = JOB_DONE;
        job_remove(&ctx->jobs, pgid);

        if (WIFEXITED(last_status))
            return WEXITSTATUS(last_status);

        if (WIFSIGNALED(last_status))
            return 128 + WTERMSIG(last_status);

        return MiniShell_ERR_UNKNOWN;
    }

    job->status = JOB_STOPPED;

    printf("\n[%d]+ Stopped %s\n",
           job->id,
           job->command);

    return MiniShell_OK;
}

static void run_process(Command *com)
{
    event_close_in_child();
    signal_reset_child();
    execvp(com->argv[0], com->argv);
    log_error("execute command failed: %s", com->argv[0]);
    _exit(127);
}
