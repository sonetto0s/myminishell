#include "test_framework.h"
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int shell_run_script(const char *script, char **output, int *status);

void test_shell_background_basic(void)
{
    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("sleep 1 &\necho done\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
        return;

    TEST_ASSERT(strstr(output, "done") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    free(output);
}


void test_shell_background_multiple(void)
{
    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("sleep 1 &\nsleep 1 &\necho done\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
        return;

    TEST_ASSERT(strstr(output, "done") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    free(output);
}

void test_shell_background_reap(void)
{
    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("sleep 1 &\nsleep 2\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
        return;

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    TEST_ASSERT(strstr(output, "process") != NULL);
    TEST_ASSERT(strstr(output, "exit 0") != NULL);
    free(output);
}


void test_shell_background_cleanup(void)
{
    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("sleep 1 &\nsleep 2\njobs\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
        return;

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    TEST_ASSERT(strstr(output, "process") != NULL);
    TEST_ASSERT(strstr(output, "exit 0") != NULL);
    TEST_ASSERT(strstr(output, "[1] sleep") == NULL);

    free(output);
}
