#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "parser.h"
#include "builtin.h"
#include "executor.h"
int dispatcher_command(Command *cmd);
#endif
