#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "command.h"
#include "shell_context.h"
int dispatcher_command(Command *cmd,ShellContext *ctx);
#endif
