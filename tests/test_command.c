#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "test_framework.h"

void test_command_init(void)
{
    Command cmd;
    command_init(&cmd);
    TEST_ASSERT_EQ(cmd.argc, 0);
    TEST_ASSERT_NULL(cmd.argv[0]);
    TEST_ASSERT_NULL(cmd.argv[MAX_ARGS - 1]);
    TEST_ASSERT_EQ(cmd.background, 0);
    TEST_ASSERT_NULL(cmd.redirect.input_file);
    TEST_ASSERT_NULL(cmd.redirect.output_file);
    TEST_ASSERT_EQ(cmd.redirect.append, 0);
    TEST_ASSERT_NULL(cmd.next);
}

void test_command_init_null(void)
{
    command_init(NULL);
    TEST_ASSERT(1);
}

void test_command_free_single(void)
{
    Command *cmd = malloc(sizeof(Command));
    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd != NULL)
    {
        command_init(cmd);
        cmd->argv[0] = strdup("echo");
        cmd->argv[1] = strdup("hello");
        cmd->argc = 2;
        cmd->redirect.input_file = strdup("input.txt");
        cmd->redirect.output_file = strdup("output.txt");
        cmd->redirect.append = 1;
        cmd->background = 1;
        command_free(cmd);
        TEST_ASSERT(1);
    }
}

void test_command_free_chain(void)
{
    Command *first = malloc(sizeof(Command));
    Command *second = malloc(sizeof(Command));
    Command *third = malloc(sizeof(Command));
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_NOT_NULL(third);

    if (first == NULL || second == NULL || third == NULL)
    {
        free(first);
        free(second);
        free(third);
        return;
    }

    command_init(first);
    command_init(second);
    command_init(third);
    first->argv[0] = strdup("ls");
    first->argc = 1;
    second->argv[0] = strdup("grep");
    second->argv[1] = strdup("txt");
    second->argc = 2;
    third->argv[0] = strdup("wc");
    third->argc = 1;
    first->next = second;
    second->next = third;
    command_free(first);
    TEST_ASSERT(1);
}

