#include "builtin.h"
#include "error.h"
#include "terminal.h"
#include "log.h"
#include "shell_context.h"
#include "builtin_table.h"
#include "system_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int builtin_cd(Command *cmd, struct ShellContext *ctx)
{
    (void)ctx;

    if (cmd->argc < 2)
    {
        const char *home = getenv("HOME");

        if (!home)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return MiniShell_ERR_UNKNOWN;
        }

        if (chdir(home) < 0)
        {
            perror("cd");
            return MiniShell_ERR_UNKNOWN;
        }

        return MiniShell_OK;
    }

    if (chdir(cmd->argv[1]) < 0)
    {
        perror("cd");
        return MiniShell_ERR_UNKNOWN;
    }

    return MiniShell_OK;
}

int builtin_pwd(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    (void)ctx;

    char buffer[100];

    if (getcwd(buffer, sizeof(buffer))) {
        printf("%s\n", buffer);
        return MiniShell_OK;
    }

    perror("getcwd");
    return MiniShell_ERR_UNKNOWN;
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

    printf("Builtin commands are:\n");

    for (size_t i = 0; i < builtin_count(); i++) {
        BuiltinEntry *entry = builtin_get(i);

        if (entry) printf(" %s\n", entry->name);
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
    if (ret != MiniShell_OK) return ret;

    system_info_print(&info);
    return MiniShell_OK;
}

int builtin_fg(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;

    Job *job = ctx->jobs.head;

    while (job) {
        if (job->status == JOB_RUNNING || job->status == JOB_STOPPED) break;
        job = job->next;
    }

    if (!job) {
        printf("fg: no job\n");
        return MiniShell_ERR_JOB;
    }

    if (terminal_set_foreground(job->pgid) < 0) {
        log_error("failed set foreground");
        return MiniShell_ERR_UNKNOWN;
    }

    if (job->status == JOB_STOPPED) {
        if (job_continue(job) < 0) {
            terminal_restore();
            return MiniShell_ERR_JOB;
        }
    }

    if (job_wait_foreground(job) < 0) {
        terminal_restore();
        return MiniShell_ERR_UNKNOWN;
    }

    if (terminal_restore() < 0) {
        log_error("failed restore terminal");
    }

    if (job->status == JOB_STOPPED) {
        printf("\n[%d]+ Stopped %s\n", job->id, job->command);
        return MiniShell_OK;
    }

    if (job->status != JOB_DONE) return MiniShell_ERR_UNKNOWN;

    int status = job_exit_status(job);
    pid_t pgid = job->pgid;

    job_remove(&ctx->jobs, pgid);

    if (status < 0) return MiniShell_ERR_UNKNOWN;

    return status;
}

int builtin_bg(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;

    Job *job = ctx->jobs.head;

    while (job) {
        if (job->status == JOB_STOPPED) break;
        job = job->next;
    }

    if (!job) {
        printf("bg: no stopped job\n");
        return MiniShell_ERR_JOB;
    }

    if (job_continue(job) < 0) {
        log_error("failed continue background job");
        return MiniShell_ERR_JOB;
    }

    printf("[%d] %s &\n", job->id, job->command);

    return MiniShell_OK;
}

int builtin_reload(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;

    if (config_load(&ctx->config, ctx->config_file) != MiniShell_OK) {
        log_error("failed reload config");
        return MiniShell_ERR_UNKNOWN;
    }

    if (ctx->config.debug) log_setlevel(LOG_DEBUG);
    else log_setlevel(LOG_INFO);

    printf("config reloaded\n");

    return MiniShell_OK;
}


