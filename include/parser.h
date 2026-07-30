#ifndef PARSER_H
#define PARSER_H


#define MAX_TOKEN 16
#define TOKEN_SIZE 64

#include "command.h"
#include "shell_context.h"

typedef enum{
    TOKEN_WORD,
    TOKEN_REDIRECT_OUT,     // >
    TOKEN_REDIRECT_APPEND,  // >>
    TOKEN_REDIRECT_IN,      // <
    TOKEN_PIPE,
    TOKEN_BACKGROUND
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

Command *parse_line(char *line, ShellContext *ctx);
void tokenize(char *line, TokenList *list);
Command *build_command(TokenList *list, ShellContext *ctx);
Command *new_command();
#endif
