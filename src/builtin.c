#include "builtin.h"
#include "error.h"
#include "terminal.h"
#include "log.h"
#include "shell_context.h"
#include "builtin_table.h"
#include "system_info.h"
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/utsname.h>

int builtin_cd(Command *cmd, struct ShellContext *ctx)
{
    (void)ctx;

    if (cmd->argc < 2)
    {
        fprintf(stderr, "你cd后面没写东西\r\n");
        return MiniShell_ERR_UNKNOWN;
    }

    if (chdir(cmd->argv[1]) == -1)
    {
        perror("chdir");
        return MiniShell_ERR_UNKNOWN;
    }

    return MiniShell_OK;
}
int builtin_pwd(Command *cmd, struct ShellContext *ctx)
{
    (void)ctx;
    (void)cmd;
    char buff[100];
    if (getcwd(buff, sizeof(buff)) != NULL)
    {
        fprintf(stdout,"%s\n",buff);
        return MiniShell_OK;
    }
    else
    {
        perror("getcwd");
        return MiniShell_ERR_UNKNOWN;
    }
}


int builtin_exit(Command *cmd, struct ShellContext *ctx)
{

    (void)cmd;
    ctx->running = 0;
    return MiniShell_OK;
}

int builtin_help(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    (void)ctx;

    printf("Builtin commands are: \n");
    for (size_t i = 0; i < builtin_count(); i++)
    {
        BuiltinEntry *entry = builtin_get(i);
        if (entry)
        {
            printf(" %s\n", entry->name);
        }
    }
    return MiniShell_OK;
}

int builtin_jobs(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    job_list(&ctx->jobs);
    return MiniShell_OK;
}
int builtin_status(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    printf("%d\n", ctx->last_exit_status);
    return MiniShell_OK;
}

int builtin_sysinfo(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    (void)ctx;
    SystemInfo info;
    int ret = system_info_collect(&info);
    if (ret != MiniShell_OK)
    {
        return ret;
    }
    system_info_print(&info);
    return MiniShell_OK;
}

int builtin_fg(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    Job *job = ctx->jobs.head;
    while (job)
    {
        if (job->status == JOB_STOPPED)
        {
            break;
        }

        job = job->next;
    }

    if (job == NULL)
    {
        printf("fg: no stopped job\n");
        return MiniShell_ERR_UNKNOWN;
    }

    if (terminal_set_foreground(job->pgid) < 0)
    {
        log_error("failed set foreground");
        return MiniShell_ERR_UNKNOWN;
    }

    if (job_continue(job) < 0)
    {
        terminal_restore();
        return MiniShell_ERR_UNKNOWN;
    }

    int status;
    pid_t ret;

    while (1)
    {
        ret = waitpid(-job->pgid, &status, WUNTRACED);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            log_error("failed wait foreground job");
            terminal_restore();
            return MiniShell_ERR_UNKNOWN;
        }
        Process *process = job->processes;
        while (process)
        {
            if (process->pid == ret)
            {
                if (WIFSTOPPED(status))
                {
                    process->status = PROCESS_STOPPED;
                }
                else if (WIFEXITED(status) || WIFSIGNALED(status))
                {
                    process->status = PROCESS_DONE;
                }
                break;
            }

            process = process->next;
        }
        if (WIFSTOPPED(status))
        {
            job->status = JOB_STOPPED;

            terminal_restore();

            printf("\n[%d]+ Stopped %s\n", job->id, job->command);

            return MiniShell_OK;
        }
        int all_done = 1;
        process = job->processes;
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
            break;
        }
    }
    terminal_restore();
    job_remove(&ctx->jobs, job->pgid);
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

int builtin_bg(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    Job *job = ctx->jobs.head;
    while (job)
    {
        if (job->status == JOB_STOPPED)
        {
            break;
        }
        job = job->next;
    }
    if (job == NULL)
    {
        printf("bg:no stopped job\n");
        return MiniShell_ERR_UNKNOWN;
    }
    if (job_continue(job) < 0)
    {
        log_error("failed continue background job");
        return MiniShell_ERR_UNKNOWN;
    }
    printf("[%d] %s &\n", job->id, job->command);
    return MiniShell_OK;
}
