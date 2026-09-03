#include "test_framework.h"
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int shell_run_script(const char *script, char **output, int *status);

void test_shell_exit_status_success(void)
{
    char *output = NULL;
    int status = 0;
    int ret = shell_run_script("/bin/true\necho $?\n", &output, &status);
    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);

    if (output == NULL)
        return;

    TEST_ASSERT(strstr(output, "0") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    free(output);
}

void test_shell_exit_status_failure(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script("/bin/false\necho $?\n", &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    if (output == NULL)
        return;

    TEST_ASSERT(strstr(output, "1") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);
    free(output);
}
