#ifndef SHELL_CONTEXT_H
#define SHELL_CONTEXT_H

#include "job.h"

typedef struct 
{
    int running;
    Job *jobs;
    int job_next_id;
    int last_exit_status;
} ShellContext;

void shell_context_init(ShellContext *ctx);

#endif
