#include "shell_context.h"
#include <stdio.h>

void shell_context_init(ShellContext *ctx)
{
    ctx->last_exit_status = 0;
    jobmanager_init(&ctx->jobs);
    ctx->running = 1;
}
