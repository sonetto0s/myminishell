#include "dispatcher.h"
#include "builtin.h"
#include "log.h"
#include "error.h"
#include "executor.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "builtin_table.h"
static int builtin_apply_redirect(Command *cmd, int *saved_out, int *saved_in)
{
    *saved_out = -1;
    *saved_in = -1;

    if (cmd->redirect.output_file)
    {
        int flags = cmd->redirect.append
                        ? (O_WRONLY | O_CREAT | O_APPEND)
                        : (O_WRONLY | O_CREAT | O_TRUNC);

        int fd = open(
            cmd->redirect.output_file,
            flags,
            0644);

        if (fd < 0)
        {
            perror("open");
            return MiniShell_ERR_OPEN;
        }

        *saved_out = dup(STDOUT_FILENO);

        if (*saved_out < 0)
        {
            close(fd);
            return MiniShell_ERR_DUP2;
        }

        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            close(fd);
            close(*saved_out);
            *saved_out = -1;
            return MiniShell_ERR_DUP2;
        }

        close(fd);
    }

    if (cmd->redirect.input_file)
    {
        int fd = open(
            cmd->redirect.input_file,
            O_RDONLY);

        if (fd < 0)
        {
            perror("open");
            return MiniShell_ERR_OPEN;
        }

        *saved_in = dup(STDIN_FILENO);

        if (*saved_in < 0)
        {
            close(fd);
            return MiniShell_ERR_DUP2;
        }

        if (dup2(fd, STDIN_FILENO) < 0)
        {
            close(fd);
            close(*saved_in);
            *saved_in = -1;
            return MiniShell_ERR_DUP2;
        }

        close(fd);
    }

    return MiniShell_OK;
}
static void builtin_restore_redirect(int saved_out, int saved_in)
{
    if (saved_out >= 0) {
        dup2(saved_out, STDOUT_FILENO);
        close(saved_out);
    }
    if (saved_in >= 0) {
        dup2(saved_in, STDIN_FILENO);
        close(saved_in);
    }
}

int dispatcher_command(Command *cmd, ShellContext *ctx)
{
    BuiltinEntry *entry = builtin_lookup(cmd->argv[0]);
    CommandType type;

    if (entry != NULL) {
        type = CMD_BUILTIN;
    } else {
        type = CMD_EXTERNAL;
    }

    switch (type) {
    case CMD_BUILTIN: {
        int saved_out, saved_in;
        int ret = builtin_apply_redirect(cmd, &saved_out, &saved_in);
        if (ret != MiniShell_OK) {
            builtin_restore_redirect(saved_out, saved_in);   /* 失败也要恢复，别漏 */
            return ret;
        }
        ret = entry->handler(cmd, ctx);
        builtin_restore_redirect(saved_out, saved_in);
        return ret;
    }
    case CMD_EXTERNAL:
        return execute_command(cmd, ctx);
    case CMD_DEVICE:
        log_error("device command is not implemented");
        return MiniShell_ERR_UNKNOWN;
    default:
        log_error("unknown command type");
        return MiniShell_ERR_UNKNOWN;
    }
}
