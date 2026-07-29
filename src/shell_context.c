#include "shell_context.h"
#include <stdio.h>

void shell_context_init(ShellContextStatus *ctx)
{
    ctx->last_exit_status = 0;
    ctx->jobs = NULL;
    ctx->job_next_id = 1;
}
