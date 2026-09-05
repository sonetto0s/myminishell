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

void test_shell_process_exit_status_failure(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "/bin/false\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 1);

    free(output);
}

void test_shell_exit_preserves_status(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "/bin/false\n"
        "exit\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 1);

    free(output);
}

void test_shell_internal_error_exit_status(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "cd /__minishell_missing_directory__\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 1);

    free(output);
}


void test_shell_command_not_found_status_127(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "__minishell_status_command_that_does_not_exist__\n"
        "exit\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 127);

    free(output);
}

void test_shell_cannot_execute_status_126(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "/tmp\n"
        "exit\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 126);

    free(output);
}

void test_shell_builtin_output_failure(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "pwd > /dev/full\n"
        "exit\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);
    if (!output) return;

    TEST_ASSERT(strstr(output, "builtin output failed") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 1);

    free(output);
}

void test_shell_builtin_output_failure_recovery(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "pwd > /dev/full\n"
        "echo alive\n"
        "exit\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);
    if (!output) return;

    TEST_ASSERT(strstr(output, "builtin output failed") != NULL);
    TEST_ASSERT(strstr(output, "alive") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    free(output);
}

void test_shell_reload_after_cd(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(
        "cd /\n"
        "reload\n"
        "exit\n",
        &output,
        &status
    );

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);
    if (!output) return;

    TEST_ASSERT(strstr(output, "config reloaded") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    free(output);
}

int shell_run_high_fd_test(char **output, int *status);

void test_shell_high_fd_fails_cleanly(void)
{
    char *output = NULL;
    int status = 0;

    int ret = shell_run_high_fd_test(&output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0)
        return;

    TEST_ASSERT_NOT_NULL(output);

    if (!output)
        return;

    TEST_ASSERT(strstr(output, "exceeds FD_SETSIZE") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 1);

    free(output);
}
