#include "dispatcher.h"
#include "builtin.h"
#include "executor.h"
#include <stdio.h>
#include "builtin_table.h"

int dispatcher_command(Command *cmd, ShellContext *ctx)
{
    BuiltinEntry *entry = builtin_lookup(cmd->argv[0]);
    if (entry != NULL)
    {
        return entry->handler(cmd,ctx);
    }
    
    return execute_command(cmd,ctx);
}