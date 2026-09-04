#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int shell_run_script(const char *script, char **output, int *status);

static const char *test_dir(void)
{
    const char *dir = getenv("MINISHELL_TEST_DIR");

    if (dir && *dir) return dir;

    return "./build/default";
}

static void make_path(char *buffer, size_t size, const char *name)
{
    snprintf(buffer, size, "%s/%s", test_dir(), name);
}

void test_shell_redirect_output(void)
{
    char file[256];
    char script[512];

    make_path(file, sizeof(file), "integration_redirect_output.txt");
    snprintf(script, sizeof(script), "echo hello > %s\n", file);

    char *output = NULL;
    int status = 0;

    unlink(file);

    int ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (!fp) {
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
    char file[256];
    char script[512];

    make_path(file, sizeof(file), "integration_redirect_append.txt");

    char *output = NULL;
    int status = 0;

    unlink(file);

    snprintf(script, sizeof(script), "echo hello > %s\n", file);

    int ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    free(output);
    output = NULL;

    snprintf(script, sizeof(script), "echo world >> %s\n", file);

    ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        free(output);
        unlink(file);
        return;
    }

    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (!fp) {
        free(output);
        unlink(file);
        return;
    }

    char buffer[256] = {0};
    size_t size = fread(buffer, 1, sizeof(buffer) - 1, fp);

    TEST_ASSERT(size > 0);

    if (size > 0) {
        TEST_ASSERT(strstr(buffer, "hello") != NULL);
        TEST_ASSERT(strstr(buffer, "world") != NULL);
    }

    fclose(fp);
    unlink(file);
    free(output);
}

void test_shell_redirect_input(void)
{
    char file[256];
    char script[512];

    make_path(file, sizeof(file), "integration_redirect_input.txt");

    FILE *fp = fopen(file, "w");
    TEST_ASSERT_NOT_NULL(fp);

    if (!fp) return;

    fputs("input_data\n", fp);
    fclose(fp);

    snprintf(script, sizeof(script), "cat < %s\n", file);

    char *output = NULL;
    int status = 0;

    int ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);

    if (ret != 0) {
        unlink(file);
        return;
    }

    TEST_ASSERT_NOT_NULL(output);

    if (!output) {
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
    char file[256];
    char script[512];

    make_path(file, sizeof(file), "integration_builtin_redirect.txt");

    snprintf(script, sizeof(script),
             "pwd > %s\n"
             "echo builtin_restore_ok\n",
             file);

    char *output = NULL;
    int status = 0;

    unlink(file);

    int ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);

    if (!output) {
        unlink(file);
        return;
    }

    TEST_ASSERT(strstr(output, "builtin_restore_ok") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (fp) {
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
    char file[256];
    char missing[256];
    char script[1024];

    make_path(file, sizeof(file), "integration_builtin_redirect_failure.txt");
    make_path(missing, sizeof(missing), "__minishell_missing_input_file__");

    snprintf(script, sizeof(script),
             "pwd > %s < %s\n"
             "echo builtin_restore_after_failure\n",
             file, missing);

    char *output = NULL;
    int status = 0;

    unlink(file);
    unlink(missing);

    int ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);

    if (!output) {
        unlink(file);
        return;
    }

    TEST_ASSERT(strstr(output, "builtin_restore_after_failure") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    FILE *fp = fopen(file, "r");
    TEST_ASSERT_NOT_NULL(fp);

    if (fp) fclose(fp);

    unlink(file);
    free(output);
}

void test_shell_external_redirect_failure(void)
{
    char missing[256];
    char script[512];

    make_path(missing, sizeof(missing), "__minishell_missing_external_input__");

    snprintf(script, sizeof(script),
             "cat < %s\n"
             "echo external_redirect_alive\n",
             missing);

    char *output = NULL;
    int status = 0;

    unlink(missing);

    int ret = shell_run_script(script, &output, &status);

    TEST_ASSERT_EQ(ret, 0);
    if (ret != 0) return;

    TEST_ASSERT_NOT_NULL(output);
    if (!output) return;

    TEST_ASSERT(strstr(output, "external_redirect_alive") != NULL);
    TEST_ASSERT(WIFEXITED(status));
    TEST_ASSERT_EQ(WEXITSTATUS(status), 0);

    free(output);
}

