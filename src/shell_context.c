#include "shell_context.h"
#include "log.h"
#include "error.h"
#include <string.h>

void shell_context_init(ShellContext *ctx)
{
    if (!ctx) return;

    memset(ctx, 0, sizeof(ShellContext));

    ctx->last_exit_status = 0;

    jobmanager_init(&ctx->jobs);
    config_init(&ctx->config);

    strncpy(ctx->config_file, "config/config.conf", sizeof(ctx->config_file) - 1);
    ctx->config_file[sizeof(ctx->config_file) - 1] = '\0';

    int ret = config_load(&ctx->config, ctx->config_file);
    if (ret != MiniShell_OK)
        log_info("failed load config");

    if (ctx->config.debug)
        log_setlevel(LOG_DEBUG);
    else
        log_setlevel(LOG_INFO);

    ctx->running = 1;
}

void shell_context_destroy(ShellContext *ctx)
{
    if (!ctx)
        return;

    ctx->running = 0;
    job_shutdown(&ctx->jobs);
}


