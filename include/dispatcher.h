#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "command.h"
#include "shell_context.h"

typedef enum
{
    CMD_BUILTIN,
    CMD_EXTERNAL,
    CMD_DEVICE
} CommandType;

int dispatcher_command(Command *cmd,ShellContext *ctx);
#endif
