#include "parser.h"

int parse_line(char *line,Command *cmd)
{
    cmd->argc = 0;
    char *token = strtok(line, " ");
    while (token != NULL)
    {
        cmd->argv[cmd->argc] = token;
        cmd->argc++;
        if(cmd->argc>=MAX_AGE)
        {
            break;
        }
        token = strtok(NULL, " ");
    }
  
    return 0;
}
 