#include "builtin.h"
#include "error.h"
#include "shell_context.h"
#include "builtin_table.h"
#include "system_info.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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