#include "parser.h"

int parse_line(char *line,Command *cmd)
{
    TokenList list;
    tokenize(line,&list);
    build_command(&list,cmd);
    return 0;
}
 
void tokenize(char *line,TokenList *list)
{
    list->count = 0;
    char *p = line;
    while (*p)
    {
        while (*p ==' ')
            p++;
        if (*p == '\0')
            break;
       
        int i = 0;
        while (*p && *p != ' ' && i < 63)
        {
            list->token[list->count].text[i++]=*p++;
        }
        list->token[list->count].text[i] = '\0';
        list->count++;
    }
}

int build_command(TokenList *list,Command *cmd)
{
    cmd->argc = 0;
    for (int i = 0; i < list->count; i++)
    {
        cmd->argv[cmd->argc++] = list->token[i].text;
        if (cmd->argc >= MAX_AGE)
            break;
    }
    cmd->argv[cmd->argc] = NULL;
    return 0;
}