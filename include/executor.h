#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"

int execute_command(Command *com);
int execute_single(Command *com);
int execute_pipeline(Command *com);
int setredirect(Command *com);
#endif
