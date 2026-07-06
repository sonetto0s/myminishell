#ifndef PARSER_H
#define PARSER_H

#define MAX_AGE 16
#define MAX_TOKEN 16
#define TOKEN_SIZE 64
#include "string.h"


typedef struct Command{
    int argc;
    char *argv[MAX_AGE];
} Command;

typedef struct {
    char text[TOKEN_SIZE];
} Token;

typedef struct{
    Token token[MAX_TOKEN];
    int count;
} TokenList;


int parse_line(char *line, Command *cmd);
void tokenize(char *line, TokenList *list);
int build_command(TokenList *list, Command *cmd);
#endif
