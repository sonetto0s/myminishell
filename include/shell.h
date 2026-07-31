#ifndef SHELL_H
#define SHELL_H

#include "shell_context.h"

typedef enum{
    SHELL_STATUS_OK = 0,
    SHELL_STATUS_ERROR = -1
}ShellStatus;

ShellStatus shell_init(ShellContext *ctx);
ShellStatus shell_run(ShellContext *ctx);
void shell_cleanup(ShellContext *ctx);

#endif
