#ifndef SHELL_CONTEXT_H
#define SHELL_CONTEXT_H

#include "job.h"

typedef struct 
{
    Job *jobs;
    int job_next_id;
    int last_exit_status;
} ShellContextStatus;

void shell_context_init(ShellContextStatus *ctx);

#endif
