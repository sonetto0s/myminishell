#include "shell.h"
#include "command.h"
#include "dispatcher.h"
#include "event.h"
#include "job.h"
#include "log.h"
#include "parser.h"
#include "sig.h"
#include "terminal.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

static int shell_normalize_status(int status)
{
    return status < 0 ? 1 : status;
}

static void shell_reset_input(ShellContext *ctx)
{
    ctx->input_length = 0;
    ctx->input_discarding = 0;
    ctx->input_buffer[0] = '\0';
}

static int shell_flush_stdout(void)
{
    while (fflush(stdout) != 0) {
        if (errno != EINTR)
            return -1;

        clearerr(stdout);
    }

    return 0;
}

static int shell_write_all(int fd, const char *buffer, size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t ret = write(fd, buffer + written, length - written);

        if (ret < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (ret == 0) {
            errno = EIO;
            return -1;
        }

        written += (size_t)ret;
    }

    return 0;
}

static int shell_print_prompt(ShellContext *ctx)
{
    char prompt[128];

    int length = snprintf(prompt, sizeof(prompt), ">>%s ", ctx->config.prompts);

    if (length < 0 || (size_t)length >= sizeof(prompt)) {
        errno = EOVERFLOW;
        return -1;
    }

    if (shell_flush_stdout() < 0)
        return -1;

    return shell_write_all(STDOUT_FILENO, prompt, (size_t)length);
}

static void shell_dispatch_input(ShellContext *ctx)
{
    if (ctx->input_length > 0 &&
        ctx->input_buffer[ctx->input_length - 1] == '\r')
        ctx->input_length--;

    ctx->input_buffer[ctx->input_length] = '\0';

    Command *cmd_list = parse_line(ctx->input_buffer, ctx);

    if (cmd_list) {
        int status = dispatcher_command(cmd_list, ctx);
        ctx->last_exit_status = shell_normalize_status(status);
        command_free(cmd_list);
    }

    shell_reset_input(ctx);
}

static int shell_read_input(ShellContext *ctx,
                            int *line_done,
                            int *eof)
{
    char ch;

    *line_done = 0;
    *eof = 0;

    ssize_t ret = read(STDIN_FILENO, &ch, 1);

    if (ret < 0) {
        if (errno == EINTR)
            return 0;

        log_error("read stdin failed: %s", strerror(errno));
        return -1;
    }

    if (ret == 0) {
        *eof = 1;

        if (!ctx->input_discarding && ctx->input_length > 0)
            shell_dispatch_input(ctx);
        else
            shell_reset_input(ctx);

        *line_done = 1;
        return 0;
    }

    if (ctx->input_discarding) {
        if (ch == '\n') {
            shell_reset_input(ctx);
            *line_done = 1;
        }

        return 0;
    }

    if (ch == '\n') {
        shell_dispatch_input(ctx);
        *line_done = 1;
        return 0;
    }

    if (ctx->input_length + 1 >= sizeof(ctx->input_buffer)) {
        fprintf(stderr, "minishell: input line is too long\n");
        ctx->last_exit_status = 2;
        ctx->input_length = 0;
        ctx->input_discarding = 1;
        return 0;
    }

    ctx->input_buffer[ctx->input_length++] = ch;
    return 0;
}

ShellStatus shell_init(ShellContext *ctx)
{
    if (!ctx)
        return SHELL_STATUS_ERROR;

    log_init();
    shell_context_init(ctx);

    if (event_init() < 0) {
        shell_context_destroy(ctx);
        return SHELL_STATUS_ERROR;
    }

    if (terminal_init() < 0) {
        event_shut();
        shell_context_destroy(ctx);
        return SHELL_STATUS_ERROR;
    }

    if (signal_init() < 0) {
        signal_shutdown();
        terminal_shutdown();
        event_shut();
        shell_context_destroy(ctx);
        return SHELL_STATUS_ERROR;
    }

    log_info(">>shell初始化成功");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(ShellContext *ctx)
{
    if (!ctx)
        return SHELL_STATUS_ERROR;

    int event_fd = event_getfd();

    if (event_fd < 0) {
        log_error("invalid event fd");
        return SHELL_STATUS_ERROR;
    }

    if (event_fd >= FD_SETSIZE) {
        log_error("event fd %d exceeds FD_SETSIZE %d",
                  event_fd, FD_SETSIZE);
        return SHELL_STATUS_ERROR;
    }

    int prompt = 1;

    while (ctx->running) {
        if (prompt) {
            if (shell_print_prompt(ctx) < 0) {
                log_error("failed write prompt");
                return SHELL_STATUS_ERROR;
            }

            prompt = 0;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(event_fd, &readfds);

        int max_fd = event_fd > STDIN_FILENO ? event_fd : STDIN_FILENO;

        int ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (ret < 0) {
            if (errno == EINTR)
                continue;

            log_error("select failed: %s", strerror(errno));
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
                shell_reset_input(ctx);

                if (shell_flush_stdout() < 0 ||
                    shell_write_all(STDOUT_FILENO, "\n", 1) < 0) {
                    log_error("failed write interrupt prompt");
                    return SHELL_STATUS_ERROR;
                }

                prompt = 1;
            }
        }

        if (!ctx->running)
            break;

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int line_done = 0;
            int eof = 0;

            if (shell_read_input(ctx, &line_done, &eof) < 0)
                return SHELL_STATUS_ERROR;

            if (line_done)
                prompt = 1;

            if (eof)
                break;
        }
    }

    return SHELL_STATUS_OK;
}

void shell_cleanup(ShellContext *ctx)
{
    shell_context_destroy(ctx);

    signal_shutdown();
    event_shut();
    terminal_shutdown();

    if (shell_flush_stdout() == 0)
        shell_write_all(STDOUT_FILENO,
                        ">>MiniShell 已退出\r\n",
                        strlen(">>MiniShell 已退出\r\n"));
}
