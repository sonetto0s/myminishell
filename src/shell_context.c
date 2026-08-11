#include "shell_context.h"
#include "log.h"
#include <string.h>
#include <stdio.h>

void shell_context_init(ShellContext *ctx)
{
    memset(ctx, 0, sizeof(ShellContext));
    ctx->last_exit_status = 0;
    jobmanager_init(&ctx->jobs);
    config_init(&ctx->config);
    config_load(&ctx->config, "config/config.conf");
    if(ctx->config.debug)
    {
        log_setlevel(LOG_DEBUG);
    }
    else
    {
        log_setlevel(LOG_INFO);
    }
    ctx->running = 1;
}

void shell_context_destroy(ShellContext *ctx)
{
    if (ctx == NULL)
        return;
    job_destroy(&ctx->jobs);
    ctx->running = 0;
}