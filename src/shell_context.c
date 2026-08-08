#include "shell_context.h"

#include <stdio.h>

void shell_context_init(ShellContext *ctx)
{
    memset(ctx, 0, sizeof(ShellContext));
    ctx->last_exit_status = 0;
    jobmanager_init(&ctx->jobs);
    config_init(&ctx->config);
    config_load(&ctx->config, "config/config.conf");
    ctx->running = 1;
}

void shell_context_destroy(ShellContext *ctx)
{
    job_destroy(&ctx->jobs);
    
    ctx->running = 0;
}