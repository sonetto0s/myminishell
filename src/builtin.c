#include "builtin.h"
#include "shell_context.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "builtin_table.h"

int builtin_cd(Command *cmd, struct ShellContext *ctx)
{
    (void)ctx;
    if (cmd->argc < 2)
    {
        fprintf(stderr, "你cd后面没写东西\r\n");
        return -1;
    }
      
    if (chdir(cmd->argv[1]) == -1)
    {
        perror("chdir");
        return -1;
    }
    
    return 0;
}
int builtin_pwd(Command *cmd, struct ShellContext *ctx)
{
    (void)ctx;
    (void)cmd;
    char buff[100];
    if (getcwd(buff, sizeof(buff)) != NULL)
    {
        fprintf(stdout,"%s\n",buff);
        return 0;
    }
    else
    {
        perror("getcwd");
        return -1;
    }
}

int builtin_exit(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    ctx->running = 0;
    return 0;
}

int builtin_job(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    job_list(&ctx->jobs);
    return 0;
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
    return 0;
}

int builtin_jobs(Command *cmd, struct ShellContext *ctx)
{
    job_list(&ctx->jobs);
    return 0;
}
int builtin_status(Command *cmd, struct ShellContext *ctx)
{
    (void)cmd;
    printf("%d\n", ctx->last_exit_status);
    return 0;
}