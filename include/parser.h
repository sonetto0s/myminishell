#ifndef PARSER_H
#define PARSER_H

#define MAX_AGE 10

#include "string.h"


typedef struct Command{
    int argc;
    char *argv[MAX_AGE];
} Command;

int parse_line(char *line, Command *cmd);
#endif
