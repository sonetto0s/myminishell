#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
#include "error.h"
#include "command.h"
#include "dispatcher.h"
#include "shell_context.h"
#include "test_framework.h"

static int dispatcher_test_path(char *buffer,
                                size_t size)
{
    const char *dir =
        getenv("MINISHELL_TEST_DIR");

    if (!dir || !*dir)
        dir = "/tmp";

    int written =
        snprintf(buffer,
                 size,
                 "%s/dispatcher_test.txt",
                 dir);

    if (written < 0 ||
        (size_t)written >= size)
        return -1;

    return 0;
}


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
    char filename[512];
    FILE *fp;
    char buffer[128] = {0};
    int ret;

    shell_context_init(&ctx);

    int path_ret =
        dispatcher_test_path(filename,
                             sizeof(filename));

    TEST_ASSERT_EQ(path_ret, 0);

    if (path_ret != 0) {
        shell_context_destroy(&ctx);
        return;
    }

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

void test_dispatcher_builtin_pipeline_rejected(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    Command *first = new_command();
    Command *second = new_command();

    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_NOT_NULL(second);

    if (!first || !second) {
        command_free(first);
        command_free(second);
        shell_context_destroy(&ctx);
        return;
    }

    first->argv[0] = strdup("pwd");
    first->argc = 1;
    first->next = second;

    second->argv[0] = strdup("wc");
    second->argv[1] = strdup("-c");
    second->argc = 2;

    TEST_ASSERT_NOT_NULL(first->argv[0]);
    TEST_ASSERT_NOT_NULL(second->argv[0]);
    TEST_ASSERT_NOT_NULL(second->argv[1]);

    if (first->argv[0] &&
        second->argv[0] &&
        second->argv[1]) {

        TEST_ASSERT_EQ(
            dispatcher_command(first, &ctx),
            MiniShell_ERR_PARSE
        );
    }

    command_free(first);
    shell_context_destroy(&ctx);
}

void test_dispatcher_builtin_background_rejected(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);

    Command *cmd = new_command();

    TEST_ASSERT_NOT_NULL(cmd);

    if (!cmd) {
        shell_context_destroy(&ctx);
        return;
    }

    cmd->argv[0] = strdup("pwd");
    cmd->argc = 1;
    cmd->background = 1;

    TEST_ASSERT_NOT_NULL(cmd->argv[0]);

    if (cmd->argv[0]) {
        TEST_ASSERT_EQ(
            dispatcher_command(cmd, &ctx),
            MiniShell_ERR_PARSE
        );
    }

    command_free(cmd);
    shell_context_destroy(&ctx);
}
