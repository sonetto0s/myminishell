#include "shell_context.h"
#include "error.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int shell_context_set_config_path(ShellContext *ctx)
{
    char cwd[SHELL_CONFIG_PATH_SIZE];

    if (!getcwd(cwd, sizeof(cwd))) {
        log_error("failed resolve current directory");
        ctx->config_file[0] = '\0';
        return -1;
    }

    int written = snprintf(ctx->config_file, sizeof(ctx->config_file),
                           "%s/config/config.conf", cwd);

    if (written < 0 || (size_t)written >= sizeof(ctx->config_file)) {
        ctx->config_file[0] = '\0';
        log_error("config path is too long");
        return -1;
    }

    return 0;
}

void shell_context_init(ShellContext *ctx)
{
    if (!ctx) return;

    memset(ctx, 0, sizeof(*ctx));
    ctx->last_exit_status = 0;

    jobmanager_init(&ctx->jobs);
    config_init(&ctx->config);

    if (shell_context_set_config_path(ctx) == 0) {
        int ret = config_load(&ctx->config, ctx->config_file);
        if (ret != MiniShell_OK)
            log_info("using default config");
    }

    log_setlevel(ctx->config.debug ? LOG_DEBUG : LOG_INFO);

    ctx->input_length = 0;
    ctx->input_discarding = 0;
    ctx->input_buffer[0] = '\0';
    ctx->running = 1;
}

void shell_context_destroy(ShellContext *ctx)
{
    if (!ctx) return;

    ctx->running = 0;
    job_shutdown(&ctx->jobs);
}


