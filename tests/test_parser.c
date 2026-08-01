#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "shell_context.h"
#include "command.h"

void test_parser()
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "ls -l";
    Command *cmd = parse_line(input,&ctx);
    if (cmd != NULL &&strcmp(cmd->argv[0], "ls") == 0 && strcmp(cmd->argv[1], "-l") == 0)
    {
        printf("[pass] parser win\n");
    }
    else
    {
        printf("[fail] parser is a loser\n");
    }
    command_free(cmd);
    shell_context_destroy(&ctx);
}