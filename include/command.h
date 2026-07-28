#ifndef COMMAND_H
#define COMMAND_H

#define MAX_ARGS 64

typedef struct
{
    char *input_file;
    char *output_file;
    int append;
} Redirection;

typedef struct Command
{
    int argc;
    char *argv[MAX_ARGS];
    int background;
    Redirection redirect;
    struct Command *next;
} Command;

void command_init(Command *cmd);
void command_free(Command *cmd);

#endif
