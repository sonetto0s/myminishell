#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "command.h"
#include "dispatcher.h"
#include "shell_context.h"
#include "test_framework.h"

void test_dispatcher_builtin(void)
{
    ShellContext ctx;
    Command *cmd;
    int ret;
    shell_context_init(&ctx);
    cmd = malloc(sizeof(Command));
    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd == NULL)
    {
        shell_context_destroy(&ctx);
        return;
    }

    command_init(cmd);
    cmd->argv[0] = strdup("pwd");
    cmd->argc = 1;
    TEST_ASSERT_NOT_NULL(cmd->argv[0]);
    if (cmd->argv[0] != NULL)
    {
        ret = dispatcher_command(cmd, &ctx);
        TEST_ASSERT_EQ(ret, MiniShell_OK);
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_dispatcher_external(void)
{
    ShellContext ctx;
    Command *cmd;
    int ret;
    shell_context_init(&ctx);
    cmd = malloc(sizeof(Command));
    TEST_ASSERT_NOT_NULL(cmd);

    if (cmd == NULL)
    {
        shell_context_destroy(&ctx);
        return;
    }

    command_init(cmd);
    cmd->argv[0] = strdup("/bin/true");
    cmd->argc = 1;

    TEST_ASSERT_NOT_NULL(cmd->argv[0]);
    if (cmd->argv[0] != NULL)
    {
        ret = dispatcher_command(cmd, &ctx);
        TEST_ASSERT_EQ(ret, MiniShell_OK);
    }
    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_dispatcher_builtin_redirect(void)
{
    ShellContext ctx;
    Command *cmd;
    const char *filename = "build/dispatcher_test.txt";
    FILE *fp;
    char buffer[128] = {0};
    int ret;
    shell_context_init(&ctx);
    cmd = malloc(sizeof(Command));
    TEST_ASSERT_NOT_NULL(cmd);

    if (cmd == NULL)
    {
        shell_context_destroy(&ctx);
        return;
    }

    command_init(cmd);
    cmd->argv[0] = strdup("pwd");
    cmd->argc = 1;
    cmd->redirect.output_file = strdup(filename);
    cmd->redirect.append = 0;
    TEST_ASSERT_NOT_NULL(cmd->argv[0]);
    TEST_ASSERT_NOT_NULL(cmd->redirect.output_file);

    if (cmd->argv[0] != NULL &&
        cmd->redirect.output_file != NULL)
    {
        ret = dispatcher_command(cmd, &ctx);
        TEST_ASSERT_EQ(ret, MiniShell_OK);
        fp = fopen(filename, "r");
        TEST_ASSERT_NOT_NULL(fp);
        if (fp != NULL)
        {
            TEST_ASSERT(fgets(buffer, sizeof(buffer), fp) != NULL);
            fclose(fp);
            TEST_ASSERT(strlen(buffer) > 0);
        }
        unlink(filename);
    }
    command_free(cmd);
    shell_context_destroy(&ctx);
}

void test_dispatcher_external_not_found(void)
{
    ShellContext ctx;
    Command *cmd;
    int ret;
    shell_context_init(&ctx);
    cmd = malloc(sizeof(Command));
    TEST_ASSERT_NOT_NULL(cmd);
    if (cmd == NULL)
    {
        shell_context_destroy(&ctx);
        return;
    }

    command_init(cmd);
    cmd->argv[0] = strdup(
        "__minishell_command_that_does_not_exist__");
    cmd->argc = 1;

    TEST_ASSERT_NOT_NULL(cmd->argv[0]);
    if (cmd->argv[0] != NULL)
    {
        ret = dispatcher_command(cmd, &ctx);
        TEST_ASSERT(ret != MiniShell_OK);
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}
