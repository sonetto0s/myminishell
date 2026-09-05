#include "parser.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKENIZE_OK 0
#define TOKENIZE_TOO_MANY -1
#define TOKENIZE_TOO_LONG -2

static int token_push(TokenList *list, TokenType type,
                      const char *text, size_t length)
{
    if (list->count >= MAX_TOKEN)
        return TOKENIZE_TOO_MANY;

    if (length >= TOKEN_SIZE)
        return TOKENIZE_TOO_LONG;

    list->token[list->count].type = type;

    memcpy(list->token[list->count].text,
           text,
           length);

    list->token[list->count].text[length] = '\0';

    list->count++;

    return TOKENIZE_OK;
}

static int is_special(char c)
{
    return c == '>' ||
           c == '<' ||
           c == '|' ||
           c == '&';
}

static Command *tokenize_fail(ShellContext *ctx, int result)
{
    if (result == TOKENIZE_TOO_LONG)
        fprintf(stderr,
                "minishell: syntax error: token is too long\n");
    else
        fprintf(stderr,
                "minishell: syntax error: too many tokens\n");

    if (ctx)
        ctx->last_exit_status = 2;

    return NULL;
}

static Command *syntax_fail(ShellContext *ctx,
                            Command *head,
                            const char *message)
{
    fprintf(stderr,
            "minishell: syntax error: %s\n",
            message);

    if (ctx)
        ctx->last_exit_status = 2;

    command_free(head);

    return NULL;
}

static Command *memory_fail(ShellContext *ctx,
                            Command *head)
{
    fprintf(stderr,
            "minishell: memory allocation failed\n");

    if (ctx)
        ctx->last_exit_status = 1;

    command_free(head);

    return NULL;
}

Command *parse_line(char *line, ShellContext *ctx)
{
    if (!line)
        return NULL;

    TokenList list;

    int result = tokenize(line, &list);

    if (result != TOKENIZE_OK)
        return tokenize_fail(ctx, result);

    if (list.count == 0)
        return NULL;

    return build_command(&list, ctx);
}

int tokenize(char *line, TokenList *list)
{
    if (!line || !list)
        return TOKENIZE_TOO_MANY;

    list->count = 0;

    char *p = line;

    while (*p) {
        while (isspace((unsigned char)*p))
            p++;

        if (*p == '\0')
            break;

        if (*p == '>') {
            if (*(p + 1) == '>') {
                int result =
                    token_push(list,
                               TOKEN_REDIRECT_APPEND,
                               p,
                               2);

                if (result != TOKENIZE_OK)
                    return result;

                p += 2;
            } else {
                int result =
                    token_push(list,
                               TOKEN_REDIRECT_OUT,
                               p,
                               1);

                if (result != TOKENIZE_OK)
                    return result;

                p++;
            }

            continue;
        }

        if (*p == '<') {
            int result =
                token_push(list,
                           TOKEN_REDIRECT_IN,
                           p,
                           1);

            if (result != TOKENIZE_OK)
                return result;

            p++;
            continue;
        }

        if (*p == '|') {
            int result =
                token_push(list,
                           TOKEN_PIPE,
                           p,
                           1);

            if (result != TOKENIZE_OK)
                return result;

            p++;
            continue;
        }

        if (*p == '&') {
            int result =
                token_push(list,
                           TOKEN_BACKGROUND,
                           p,
                           1);

            if (result != TOKENIZE_OK)
                return result;

            p++;
            continue;
        }

        char *start = p;

        while (*p &&
               !isspace((unsigned char)*p) &&
               !is_special(*p))
            p++;

        size_t length = (size_t)(p - start);

        int result =
            token_push(list,
                       TOKEN_WORD,
                       start,
                       length);

        if (result != TOKENIZE_OK)
            return result;
    }

    return TOKENIZE_OK;
}

Command *build_command(TokenList *list, ShellContext *ctx)
{
    if (!list)
        return NULL;

    Command *head = new_command();

    if (!head)
        return memory_fail(ctx, NULL);

    Command *current = head;

    for (int i = 0; i < list->count; i++) {
        switch (list->token[i].type) {
        case TOKEN_WORD:
            if (current->argc >= MAX_ARGS - 1)
                return syntax_fail(ctx,
                                   head,
                                   "too many arguments");

            if (strcmp(list->token[i].text, "$?") == 0) {
                char buffer[64];

                int status =
                    ctx
                        ? ctx->last_exit_status
                        : 0;

                snprintf(buffer,
                         sizeof(buffer),
                         "%d",
                         status);

                current->argv[current->argc] =
                    strdup(buffer);
            } else {
                current->argv[current->argc] =
                    strdup(list->token[i].text);
            }

            if (!current->argv[current->argc])
                return memory_fail(ctx, head);

            current->argc++;

            break;

        case TOKEN_REDIRECT_IN:
            if (i + 1 >= list->count ||
                list->token[i + 1].type != TOKEN_WORD)
                return syntax_fail(
                    ctx,
                    head,
                    "expected filename after '<'"
                );

            if (current->redirect.input_file)
                return syntax_fail(
                    ctx,
                    head,
                    "duplicate input redirection"
                );

            current->redirect.input_file =
                strdup(list->token[i + 1].text);

            if (!current->redirect.input_file)
                return memory_fail(ctx, head);

            i++;

            break;

        case TOKEN_REDIRECT_OUT:
            if (i + 1 >= list->count ||
                list->token[i + 1].type != TOKEN_WORD)
                return syntax_fail(
                    ctx,
                    head,
                    "expected filename after '>'"
                );

            if (current->redirect.output_file)
                return syntax_fail(
                    ctx,
                    head,
                    "duplicate output redirection"
                );

            current->redirect.output_file =
                strdup(list->token[i + 1].text);

            if (!current->redirect.output_file)
                return memory_fail(ctx, head);

            current->redirect.append = 0;

            i++;

            break;

        case TOKEN_REDIRECT_APPEND:
            if (i + 1 >= list->count ||
                list->token[i + 1].type != TOKEN_WORD)
                return syntax_fail(
                    ctx,
                    head,
                    "expected filename after '>>'"
                );

            if (current->redirect.output_file)
                return syntax_fail(
                    ctx,
                    head,
                    "duplicate output redirection"
                );

            current->redirect.output_file =
                strdup(list->token[i + 1].text);

            if (!current->redirect.output_file)
                return memory_fail(ctx, head);

            current->redirect.append = 1;

            i++;

            break;

        case TOKEN_BACKGROUND:
            if (i != list->count - 1)
                return syntax_fail(
                    ctx,
                    head,
                    "'&' must be the last token"
                );

            if (head->background)
                return syntax_fail(
                    ctx,
                    head,
                    "duplicate background operator"
                );

            head->background = 1;

            break;

        case TOKEN_PIPE:
            if (current->argc == 0)
                return syntax_fail(
                    ctx,
                    head,
                    "missing command near '|'"
                );

            if (i + 1 >= list->count ||
                list->token[i + 1].type != TOKEN_WORD)
                return syntax_fail(
                    ctx,
                    head,
                    "expected command after '|'"
                );

            Command *next = new_command();

            if (!next)
                return memory_fail(ctx, head);

            current->next = next;
            current = next;

            break;
        }
    }

    for (Command *iterator = head;
         iterator;
         iterator = iterator->next) {
        if (iterator->argc == 0)
            return syntax_fail(
                ctx,
                head,
                "missing command near '|'"
            );

        iterator->argv[iterator->argc] = NULL;
    }

    return head;
}

Command *new_command(void)
{
    Command *cmd = malloc(sizeof(Command));

    if (!cmd)
        return NULL;

    command_init(cmd);

    return cmd;
}
