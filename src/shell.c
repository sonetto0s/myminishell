#include "shell.h"
#include "utils.h"
#include "parser.h"
#include "dispatcher.h"
#include "command.h"
#include "sig.h"
#include "event.h"
#include "job.h"
#include "log.h"
#include "error.h"
#include "terminal.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

ShellStatus shell_init(ShellContext *ctx)
{
    if (!ctx)
        return SHELL_STATUS_ERROR;
        
    setvbuf(stdin, NULL, _IONBF, 0);
    log_init();

    shell_context_init(ctx);

    if (event_init() < 0) {
        shell_context_destroy(ctx);
        return SHELL_STATUS_ERROR;
    }

    if (signal_init() < 0) {
        event_shut();
        shell_context_destroy(ctx);
        return SHELL_STATUS_ERROR;
    }

    if (terminal_init() < 0) {
        event_shut();
        shell_context_destroy(ctx);
        return SHELL_STATUS_ERROR;
    }

    log_info(">>shell初始化成功");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(ShellContext *ctx)
{
    if (!ctx) return SHELL_STATUS_ERROR;

    int event_fd = event_getfd();
    if (event_fd < 0) return SHELL_STATUS_ERROR;

    int prompt = 1;

    while (ctx->running) {
        if (prompt) {
            printf(">>%s ", ctx->config.prompts);
            fflush(stdout);
            prompt = 0;
        }

        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(event_fd, &readfds);

        int max_fd = event_fd > STDIN_FILENO ? event_fd : STDIN_FILENO;

        int ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (ret < 0) {
            if (errno == EINTR) continue;

            log_error("select failed");
            return SHELL_STATUS_ERROR;
        }

        if (FD_ISSET(event_fd, &readfds)) {
            if (event_drain() < 0) {
                log_error("failed drain event pipe");
                return SHELL_STATUS_ERROR;
            }

            int events = signal_take_events();

            if (events & SIGNAL_EVENT_CHILD) {
                job_reap(&ctx->jobs);
                job_cleanup_done(&ctx->jobs);
            }

            if (events & SIGNAL_EVENT_INTERRUPT) {
                clearerr(stdin);
                printf("\n");
                fflush(stdout);
                prompt = 1;
            }
        }

        if (!ctx->running) break;

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            Command *cmd_list = NULL;
            char *line = NULL;
            size_t line_capacity = 0;

            ssize_t readline = getline(&line, &line_capacity, stdin);

            if (readline == -1) {
                free(line);

                if (feof(stdin)) break;

                if (errno == EINTR) {
                    clearerr(stdin);
                    prompt = 1;
                    continue;
                }

                return SHELL_STATUS_ERROR;
            }

            trim_line(line);
            cmd_list = parse_line(line, ctx);

            if (cmd_list) {
                int status = dispatcher_command(cmd_list, ctx);
                ctx->last_exit_status = status;

                if (status < 0) {
                    fprintf(stderr, "%s\n", minishell_error_string(status));
                }

                command_free(cmd_list);
            }

            free(line);
            prompt = 1;
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


