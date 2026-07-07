#include "command.h"
#include <stddef.h>

void command_init(Command *cmd)
{
    if (cmd == NULL)
        return;

    cmd->argc = 0;
    for (int i = 0; i < MAX_ARGS; i++)
    {
        cmd->argv[i] = NULL;
    }
    cmd->redirect.output_file = NULL;
    cmd->redirect.input_file = NULL;
    cmd->redirect.append = 0;
}
