#include <string.h>
#include "command.h"
#include "parser.h"
#include "shell_context.h"
#include "test_framework.h"
#include <stdio.h>

void test_parser_basic(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    char input[] = "ls -l";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NOT_NULL(cmd);

    if (cmd != NULL)
    {
        TEST_ASSERT_EQ(cmd->argc, 2);
        TEST_ASSERT_STR_EQ(cmd->argv[0], "ls");
        TEST_ASSERT_STR_EQ(cmd->argv[1], "-l");
        TEST_ASSERT_NULL(cmd->argv[2]);
    }
    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_pipeline(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "ls | grep txt";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NOT_NULL(cmd);

    if (cmd != NULL)
    {
        TEST_ASSERT_STR_EQ(cmd->argv[0], "ls");
        TEST_ASSERT_NULL(cmd->redirect.input_file);
        TEST_ASSERT_NULL(cmd->redirect.output_file);
        TEST_ASSERT_NOT_NULL(cmd->next);

        if (cmd->next != NULL)
        {
            TEST_ASSERT_STR_EQ(cmd->next->argv[0], "grep");
            TEST_ASSERT_STR_EQ(cmd->next->argv[1], "txt");
            TEST_ASSERT_NULL(cmd->next->next);
        }
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_redirect_out(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "echo hello > output.txt";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd != NULL)
    {
        TEST_ASSERT_EQ(cmd->argc, 2);
        TEST_ASSERT_STR_EQ(cmd->argv[0], "echo");
        TEST_ASSERT_STR_EQ(cmd->argv[1], "hello");
        TEST_ASSERT_STR_EQ(cmd->redirect.output_file, "output.txt");
        TEST_ASSERT_EQ(cmd->redirect.append, 0);
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_redirect_append(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "echo hello >> output.txt";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd != NULL)
    {
        TEST_ASSERT_STR_EQ(cmd->redirect.output_file, "output.txt");
        TEST_ASSERT_EQ(cmd->redirect.append, 1);
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_redirect_in(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "cat < input.txt";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd != NULL)
    {
        TEST_ASSERT_EQ(cmd->argc, 1);
        TEST_ASSERT_STR_EQ(cmd->argv[0], "cat");
        TEST_ASSERT_STR_EQ(cmd->redirect.input_file, "input.txt");
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_background(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "sleep 5 &";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd != NULL)
    {
        TEST_ASSERT_EQ(cmd->background, 1);
        TEST_ASSERT_STR_EQ(cmd->argv[0], "sleep");
        TEST_ASSERT_STR_EQ(cmd->argv[1], "5");
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_exit_status(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    ctx.last_exit_status = 42;

    char input[] = "echo $?";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NOT_NULL(cmd);

    if (cmd != NULL)
    {
        TEST_ASSERT_EQ(cmd->argc, 2);
        TEST_ASSERT_STR_EQ(cmd->argv[0], "echo");
        TEST_ASSERT_STR_EQ(cmd->argv[1], "42");
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_empty_input(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NULL(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_invalid_pipe(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    char input[] = "ls |";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NULL(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_invalid_redirect(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "echo hello >";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NULL(cmd);
    shell_context_destroy(&ctx);
}

void test_parser_duplicate_redirect_out(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "echo hello > first.txt > second.txt";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NULL(cmd);
    TEST_ASSERT_EQ(ctx.last_exit_status, 2);
    shell_context_destroy(&ctx);
}

void test_parser_duplicate_redirect_append(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "echo hello >> first.txt > second.txt";
    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NULL(cmd);
    TEST_ASSERT_EQ(ctx.last_exit_status, 2);
    shell_context_destroy(&ctx);
}

void test_parser_duplicate_redirect_in(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    char input[] = "cat < first.txt < second.txt";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NULL(cmd);
    TEST_ASSERT_EQ(ctx.last_exit_status, 2);
    shell_context_destroy(&ctx);
}

void test_parser_redirect_per_pipeline_command(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "cat < input.txt | grep hello > output.txt";
    Command *cmd = parse_line(input, &ctx);
    TEST_ASSERT_NOT_NULL(cmd);

    if (cmd != NULL)
    {
        TEST_ASSERT_STR_EQ(cmd->argv[0], "cat");
        TEST_ASSERT_STR_EQ(cmd->redirect.input_file, "input.txt");
        TEST_ASSERT_NULL(cmd->redirect.output_file);
        TEST_ASSERT_NOT_NULL(cmd->next);
        if (cmd->next != NULL)
        {
            TEST_ASSERT_STR_EQ(cmd->next->argv[0], "grep");
            TEST_ASSERT_STR_EQ(cmd->next->argv[1], "hello");
            TEST_ASSERT_NULL(cmd->next->redirect.input_file);
            TEST_ASSERT_STR_EQ(cmd->next->redirect.output_file, "output.txt");
        }
    }
    command_free(cmd);
    shell_context_destroy(&ctx);
}


void test_parser_token_too_long(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    char input[TOKEN_SIZE + 1];

    memset(input, 'a', TOKEN_SIZE);
    input[TOKEN_SIZE] = '\0';

    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NULL(cmd);
    TEST_ASSERT_EQ(ctx.last_exit_status, 2);

    shell_context_destroy(&ctx);
}

void test_parser_too_many_tokens(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    char input[1024] = {0};

    size_t used = 0;

    for (int i = 0; i < MAX_TOKEN + 1; i++) {
        int written = snprintf(input + used,
                               sizeof(input) - used,
                               "x%s",
                               i == MAX_TOKEN ? "" : " ");

        if (written < 0 ||
            (size_t)written >= sizeof(input) - used) {
            break;
        }

        used += (size_t)written;
    }

    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NULL(cmd);
    TEST_ASSERT_EQ(ctx.last_exit_status, 2);

    shell_context_destroy(&ctx);
}

void test_parser_background_must_be_last(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    char input[] = "sleep 1 & echo bad";

    Command *cmd = parse_line(input, &ctx);

    TEST_ASSERT_NULL(cmd);
    TEST_ASSERT_EQ(ctx.last_exit_status, 2);

    shell_context_destroy(&ctx);
}
