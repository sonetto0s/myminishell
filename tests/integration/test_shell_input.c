#include "test_framework.h"
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int shell_run_fragmented_input_test(char **output, int *status);

void test_shell_fragmented_input_sigchld(void)
{
    char *output = NULL;
    int status = 0;

    int ret =
        shell_run_fragmented_input_test(
            &output,
            &status
        );

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);

    if (!output)
        return;

    TEST_ASSERT(strstr(output, "PARTIAL") != NULL);
    TEST_ASSERT(strstr(output, "PART\n") == NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    free(output);
}
