#include "dispatcher.h"
#include "builtin.h"
#include "executor.h"
#include <stdio.h>

int dispatcher_command(Command *cmd)
{
    BuiltinEntry *entry = builtin_lookup(cmd->argv[0]);
    if (entry != NULL)
    {
        return entry->handler(cmd);
    }

    return execute_command(cmd);
}