#include "test_framework.h"
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int shell_run_script(const char *script, char **output, int *status);

void test_shell_pipeline_two_commands(void)
{
    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("echo hello | wc -c\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);

    if (output == NULL)
        return;

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    TEST_ASSERT(strstr(output, "6") != NULL);
    free(output);
}

void test_shell_pipeline_three_commands(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script("printf a\\nb\\nc\\n | grep . | wc -l\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
        return;

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    TEST_ASSERT(strstr(output, "3") != NULL);
    free(output);
}
