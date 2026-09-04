#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int shell_run_script(const char *script, char **output, int *status);

void test_shell_redirect_output(void)
{
    const char *file = "build/default/integration_redirect_output.txt";
    char *output = NULL;
    int status = 0;
    unlink(file);
    int ret = shell_run_script("echo hello > build/default/integration_redirect_output.txt\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (fp == NULL)
    {
        free(output);
        return;
    }

    char buffer[128] = {0};
    TEST_ASSERT(fgets(buffer, sizeof(buffer), fp) != NULL);
    TEST_ASSERT(strstr(buffer, "hello") != NULL);
    fclose(fp);
    unlink(file);
    free(output);
}

void test_shell_redirect_append(void)
{
    const char *file = "build/default/integration_redirect_append.txt";
    char *output = NULL;
    int status = 0;

    unlink(file);

    int ret = shell_run_script("echo hello > build/default/integration_redirect_append.txt\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    free(output);
    output = NULL;
    ret = shell_run_script("echo world >> build/default/integration_redirect_append.txt\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
    {
        free(output);
        unlink(file);
        return;
    }

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);
    if (fp == NULL)
    {
        free(output);
        unlink(file);
        return;
    }

    char buffer[256] = {0};
    size_t size = fread(buffer, 1, sizeof(buffer) - 1, fp);
    TEST_ASSERT(size > 0);

    if (size > 0)
    {
        TEST_ASSERT(strstr(buffer, "hello") != NULL);
        TEST_ASSERT(strstr(buffer, "world") != NULL);
    }

    fclose(fp);
    unlink(file);
    free(output);
}

void test_shell_redirect_input(void)
{
    const char *file = "build/default/integration_redirect_input.txt";
    FILE *fp = fopen(file, "w");
    TEST_ASSERT_NOT_NULL(fp);

    if (fp == NULL)
        return;

    fputs("input_data\n", fp);
    fclose(fp);

    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("cat < build/default/integration_redirect_input.txt\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
    {
        unlink(file);
        return;
    }

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
    {
        unlink(file);
        return;
    }

    TEST_ASSERT(strstr(output, "input_data") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    unlink(file);
    free(output);
}

void test_shell_builtin_redirect_restore_success(void)
{
    const char *file = "build/default/integration_builtin_redirect.txt";
    char *output = NULL;
    int status = 0;

    unlink(file);
    int ret = shell_run_script("pwd > build/default/integration_builtin_redirect.txt\n"
                               "echo builtin_restore_ok\n",
                               &output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
    {
        unlink(file);
        return;
    }

    TEST_ASSERT(strstr(output, "builtin_restore_ok") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (fp != NULL)
    {
        char buffer[256] = {0};

        TEST_ASSERT(fgets(buffer, sizeof(buffer), fp) != NULL);
        TEST_ASSERT(strlen(buffer) > 0);

        fclose(fp);
    }
    unlink(file);
    free(output);
}

void test_shell_builtin_redirect_restore_failure(void)
{
    const char *file = "build/default/integration_builtin_redirect_failure.txt";
    const char *missing = "build/default/__minishell_missing_input_file__";
    char *output = NULL;
    int status = 0;

    unlink(file);
    unlink(missing);

    int ret = shell_run_script(
        "pwd > build/default/integration_builtin_redirect_failure.txt "
        "< build/default/__minishell_missing_input_file__\n"
        "echo builtin_restore_after_failure\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;
    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
    {
        unlink(file);
        return;
    }

    TEST_ASSERT(strstr(output, "builtin_restore_after_failure") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (fp != NULL)
        fclose(fp);

    unlink(file);
    free(output);
}

void test_shell_external_redirect_failure(void)
{
    const char *missing = "build/default/__minishell_missing_external_input__";
    char *output = NULL;
    int status = 0;

    unlink(missing);

    int ret = shell_run_script("cat < build/default/__minishell_missing_external_input__\n"
                               "echo external_redirect_alive\n",
                               &output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);

    if (output == NULL)
        return;

    TEST_ASSERT(strstr(output, "external_redirect_alive") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    free(output);
}
