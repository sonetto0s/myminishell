#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Command *parse_line(char *line,ShellContext *ctx)
{
    TokenList list;
    tokenize(line,&list);
    if (list.count == 0)
        return NULL;
    return build_command(&list,ctx);
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
        if (*p == '|')
        {
            list->token[list->count].type = TOKEN_PIPE;
            strcpy(list->token[list->count].text, "|");
            list->count++;
            p++;
            continue;
        }
        if (*p == '&')
        {
            list->token[list->count].type = TOKEN_BACKGROUND;
            strcpy(list->token[list->count].text, "&");
            list->count++;
            p++;
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

Command *build_command(TokenList *list,ShellContext *ctx)
{
    Command *head = NULL;
    Command *current = NULL;
    head = new_command();
    if (head == NULL)
    {
        return NULL;
    }
    current = head;
    for (int i = 0; i < list->count; i++)
    {
        switch (list->token[i].type)
           {
            case TOKEN_WORD:
                if (current->argc >= MAX_ARGS)
                    goto fail;

                if (strcmp(list->token[i].text, "$?") == 0)
                {
                    char buffer[64];
                    snprintf(buffer, sizeof(buffer), "%d", ctx->last_exit_status);
                    current->argv[current->argc] = strdup(buffer);
                    
                }
                else
                {
                    current->argv[current->argc] = strdup(list->token[i].text);
                }
                if (current->argv[current->argc] == NULL)
                    goto fail;
                current->argc++;
                break;
            case TOKEN_REDIRECT_IN:
                if (i + 1 >= list->count)
                    goto fail;
                current->redirect.input_file = strdup(list->token[i + 1].text);
                if (current->redirect.input_file == NULL)
                    goto fail;
                i++;
                break;
            case TOKEN_REDIRECT_OUT:
                if (i + 1 >= list->count)
                    goto fail;
                current->redirect.output_file = strdup(list->token[i + 1].text);
                if (current->redirect.output_file == NULL)
                    goto fail;
                current->redirect.append = 0;
                i++;
                break;
            case TOKEN_REDIRECT_APPEND:
                if (i + 1 >= list->count)
                    goto fail;
                current->redirect.output_file = strdup(list->token[i + 1].text);
                if (current->redirect.output_file == NULL)
                    goto fail;
                current->redirect.append = 1;
                i++;
                break;
            case TOKEN_BACKGROUND:
                current->background = 1;
                break;
            case TOKEN_PIPE:
                if (i + 1 >= list->count)
                    goto fail;
                Command *new = new_command();
                if (new == NULL)
                    goto fail;
                current->next = new;
                current = new;
                break;
           }
    }
    Command *iterator = head;
    while(iterator)
    {
        iterator->argv[iterator->argc] = NULL;
        iterator = iterator->next;
    }
    return head;
    fail:
        command_free(head);
        return NULL;
}

Command *new_command()
{
    Command *cmd = malloc(sizeof(Command));
    if (cmd == NULL)
        return NULL;
    command_init(cmd);
    return cmd;
}
