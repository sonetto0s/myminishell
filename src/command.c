#include "command.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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
    cmd->background = 0;
    cmd->next = NULL;
}

void command_free(Command *cmd)
{
    while(cmd)
    {
        Command *next = cmd->next;
        for (int i = 0; i < cmd->argc; i++)
        {
            free(cmd->argv[i]);
            cmd->argv[i] = NULL;
        }
        free(cmd->redirect.output_file);
        free(cmd->redirect.input_file);
        free(cmd);
        cmd = next;
    }
}