#include "config.h"
#include "error.h"
#include "test_framework.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void test_config_init(void)
{
    MiniShellConfig config;
    config_init(&config);

    TEST_ASSERT_STR_EQ(config.prompts, "MiniShell");
    TEST_ASSERT_EQ(config.max_job, 64);
    TEST_ASSERT_EQ(config.debug, 0);
}

void test_config_parse_line(void)
{
    MiniShellConfig config;
    config_init(&config);

    char prompt_line[] = "prompts=TestShell";
    char max_job_line[] = "max_job=128";
    char debug_line[] = "debug=1";

    TEST_ASSERT_EQ(config_parse_line(prompt_line, &config), MiniShell_OK);
    TEST_ASSERT_EQ(config_parse_line(max_job_line, &config), MiniShell_OK);
    TEST_ASSERT_EQ(config_parse_line(debug_line, &config), MiniShell_OK);
    TEST_ASSERT_STR_EQ(config.prompts, "TestShell");
    TEST_ASSERT_EQ(config.max_job, 128);
    TEST_ASSERT_EQ(config.debug, 1);
}

void test_config_invalid_max_job(void)
{
    MiniShellConfig config;
    config_init(&config);

    char zero[] = "max_job=0";
    TEST_ASSERT_EQ(config_parse_line(zero, &config), MiniShell_ERR_PARSE);
    TEST_ASSERT_EQ(config.max_job, 64);

    char negative[] = "max_job=-1";
    TEST_ASSERT_EQ(config_parse_line(negative, &config), MiniShell_ERR_PARSE);
    TEST_ASSERT_EQ(config.max_job, 64);

    char text[] = "max_job=abc";
    TEST_ASSERT_EQ(config_parse_line(text, &config), MiniShell_ERR_PARSE);
    TEST_ASSERT_EQ(config.max_job, 64);

    char mixed[] = "max_job=12abc";
    TEST_ASSERT_EQ(config_parse_line(mixed, &config), MiniShell_ERR_PARSE);
    TEST_ASSERT_EQ(config.max_job, 64);

    char valid[] = "max_job=32";
    TEST_ASSERT_EQ(config_parse_line(valid, &config), MiniShell_OK);
    TEST_ASSERT_EQ(config.max_job, 32);
}

void test_config_load(void)
{
    const char *filename = "/tmp/minishell_test_config.conf";
    FILE *fp = fopen(filename, "w");
    TEST_ASSERT_NOT_NULL(fp);

    if (!fp) return;

    fprintf(fp, "prompts=TestShell\n");
    fprintf(fp, "max_job=128\n");
    fprintf(fp, "debug=1\n");
    fclose(fp);

    MiniShellConfig config;
    config_init(&config);

    int result = config_load(&config, filename);
    TEST_ASSERT_EQ(result, MiniShell_OK);
    TEST_ASSERT_STR_EQ(config.prompts, "TestShell");
    TEST_ASSERT_EQ(config.max_job, 128);
    TEST_ASSERT_EQ(config.debug, 1);

    unlink(filename);
}

void test_config_load_missing_file(void)
{
    MiniShellConfig config;
    config_init(&config);

    int result = config_load(&config, "/tmp/minishell_missing_config_file.conf");
    TEST_ASSERT_EQ(result, MiniShell_ERR_OPEN);
}

void test_config_transaction_failure(void)
{
    const char *filename = "/tmp/minishell_test_config_invalid.conf";
    FILE *fp = fopen(filename, "w");
    TEST_ASSERT_NOT_NULL(fp);

    if (!fp) return;

    fprintf(fp, "prompts=ChangedShell\n");
    fprintf(fp, "max_job=abc\n");
    fprintf(fp, "debug=1\n");
    fclose(fp);

    MiniShellConfig config;
    config_init(&config);
    strcpy(config.prompts, "StableShell");
    config.max_job = 32;
    config.debug = 0;

    int result = config_load(&config, filename);
    TEST_ASSERT_EQ(result, MiniShell_ERR_PARSE);
    TEST_ASSERT_STR_EQ(config.prompts, "StableShell");
    TEST_ASSERT_EQ(config.max_job, 32);
    TEST_ASSERT_EQ(config.debug, 0);

    unlink(filename);
}

void test_config_directory_load_failure(void)
{
    MiniShellConfig config;
    config_init(&config);

    int result = config_load(&config, "/tmp");
    TEST_ASSERT(result != MiniShell_OK);
}
