#ifndef SHELL_CONTEXT_H
#define SHELL_CONTEXT_H

#include "job.h"
#include "config.h"

typedef struct ShellContext
{
    int running;
    JobManager jobs;
    int last_exit_status;
    MiniShellConfig config;
    char config_file[128];
} ShellContext;

void shell_context_init(ShellContext *ctx);
void shell_context_destroy(ShellContext *ctx);

#endif
