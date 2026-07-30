#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"
#include "shell_context.h"

int execute_command(Command *com, ShellContext *ctx);
int execute_single(Command *com, ShellContext *ctx);
int execute_pipeline(Command *com);
int setredirect(Command *com);
#endif
