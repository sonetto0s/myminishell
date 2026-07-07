#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int parse_line(char *line,Command *cmd)
{
    TokenList list;
    tokenize(line,&list);
    return build_command(&list, cmd);
}
 
void tokenize(char *line,TokenList *list)
{
    list->count = 0;
    char *p = line;
    while (*p)
    {
        while (*p == ' ')
            p++;

        if (*p == '\0')
            break;

        if (*p == '>')
        {
            if (*(p + 1) == '>')
            {
                list->token[list->count].type = TOKEN_REDIRECT_APPEND;
                strcpy(list->token[list->count].text, ">>");
                list->count++;
                p += 2;
            }
            else
            {
                list->token[list->count].type = TOKEN_REDIRECT_OUT;
                strcpy(list->token[list->count].text, ">");
                list->count++;
                p++;
            }
            continue;
        }
        if (*p == '<')
        {
            list->token[list->count].type= TOKEN_REDIRECT_IN;
            strcpy(list->token[list->count].text, "<");
            p++;
            list->count++;
            continue;
        }
     
    
        list->token[list->count].type = TOKEN_WORD;
        
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
        switch(list->token[i].type)
       {
        case TOKEN_WORD:
            if (cmd->argc >= MAX_ARGS)
                return -1;
            cmd->argv[cmd->argc++] = list->token[i].text;
            break;
        case TOKEN_REDIRECT_IN:
            if (i + 1 >= list->count)
                return -1;
            cmd->redirect.input_file = list->token[i + 1].text;
            i++;
            break;
        case TOKEN_REDIRECT_OUT:
            if (i + 1 >= list->count)
                return -1;
            cmd->redirect.output_file = list->token[i + 1].text;
            cmd->redirect.append = 0;
            i++;
            break;
        case TOKEN_REDIRECT_APPEND:
            if (i + 1 >= list->count)
                return -1;
            cmd->redirect.output_file = list->token[i + 1].text;
            cmd->redirect.append = 1;
            i++;
            break;
       }
       

       
    }
    cmd->argv[cmd->argc] = NULL;
  
    return 0;
}

