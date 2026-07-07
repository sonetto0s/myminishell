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
    Redirection redirect;
} Command;

void command_init(Command *cmd);
#endif
