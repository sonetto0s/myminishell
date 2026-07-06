#ifndef SHELL_H
#define SHELL_H

typedef enum{
    SHELL_STATUS_OK = 0,
    SHELL_STATUS_ERROR = -1
}ShellStatus;

#include "utils.h"
#include "executor.h"
#include "parser.h"
#include <stdio.h>

ShellStatus shell_init(void);
ShellStatus shell_run(void);
void shell_cleanup(void);

#endif
