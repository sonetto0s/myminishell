#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include "shell.h"
#include "utils.h"
#include "parser.h"
#include "dispatcher.h"
#include "executor.h"
#include "command.h"
#include "sig.h"
#include "event.h"
#include "job.h"
#include "log.h"
#include "error.h"

ShellStatus shell_init(ShellContext *ctx)
{
    log_init();
    shell_context_init(ctx);
    if (event_init() < 0)
    {
        return SHELL_STATUS_ERROR;
    }
    signal_init(ctx);
    log_info(">>shell初始化成功");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(ShellContext *ctx)
{
    int prompt = 1;
    int event_fd = event_getfd();
    while (ctx->running)
    {
        if (prompt)
        {
            printf(">>%s ",ctx->config.prompts);
            fflush(stdout);
            prompt = 0;
        }
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(event_fd, &readfds);
        int max_fd = event_fd > STDIN_FILENO ? event_fd : STDIN_FILENO;
        int ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            log_error("select failed");
            break;
        }
        if (FD_ISSET(event_fd, &readfds))
        {
            char buffer[8];
            ssize_t n = read(event_fd, buffer, sizeof(buffer));
            if (n < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                log_error("failed read fd");
                break;
            }
            if (n == 0)
            {
                log_error("fd closed unexpectedly");
                break;
            }
            job_reap(&ctx->jobs);
        }
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            Command *cmd_list = NULL;
            char *line = NULL;
            size_t line_capacity = 0;
            ssize_t readline = getline(&line, &line_capacity, stdin);
            if (readline == -1)
            {
                free(line);
                if (feof(stdin))
                {
                    break;
                }
                if (errno == EINTR)
                {
                    clearerr(stdin);
                    continue;
                }
                break;
            }

            trim_line(line);
            cmd_list = parse_line(line, ctx);
            if (cmd_list != NULL)
            {
                int status = dispatcher_command(cmd_list, ctx);
                ctx->last_exit_status = status;
                if (status < 0)
                {
                    if (status == -EINTR)
                    {
                        command_free(cmd_list);
                        free(line);
                        continue;
                    }
                    
                    printf("%s\n", Minishellerror_string(status));
                }
                command_free(cmd_list);
                prompt = 1;
            }
            free(line);
        }
    }
    return SHELL_STATUS_OK;
}

void shell_cleanup(ShellContext *ctx)
{
    shell_context_destroy(ctx);
    event_shut();
    printf(">>MiniShell 已退出\r\n");
}


