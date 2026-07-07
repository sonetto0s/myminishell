#ifndef PARSER_H
#define PARSER_H


#define MAX_TOKEN 16
#define TOKEN_SIZE 64

#include "command.h"

typedef enum{
    TOKEN_WORD,
    TOKEN_REDIRECT_OUT,     // >
    TOKEN_REDIRECT_APPEND,  // >>
    TOKEN_REDIRECT_IN       // <
}TokenType;

typedef struct
{
    char text[TOKEN_SIZE];
    TokenType type;
} Token;

typedef struct{
    Token token[MAX_TOKEN];
    int count;
} TokenList;


int parse_line(char *line, Command *cmd);
void tokenize(char *line, TokenList *list);
int build_command(TokenList *list, Command *cmd);
#endif
