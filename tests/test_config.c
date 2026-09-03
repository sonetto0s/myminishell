#include "config.h"
#include "test_framework.h"
#include "error.h"
#include <string.h>

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
    char line1[] = "prompts=TestShell";
    char line2[] = "max_job=128";
    char line3[] = "debug=1";

    config_init(&config);
    config_parse_line(line1, &config);
    config_parse_line(line2, &config);
    config_parse_line(line3, &config);
    TEST_ASSERT_STR_EQ(config.prompts, "TestShell");
    TEST_ASSERT_EQ(config.max_job, 128);
    TEST_ASSERT_EQ(config.debug, 1);
}

void test_config_load(void)
{
    MiniShellConfig config;
    config_init(&config);
    int result = config_load(&config, "tests/test_config.conf");
    TEST_ASSERT_EQ(result, MiniShell_OK);
    TEST_ASSERT_STR_EQ(config.prompts, "TestShell");
    TEST_ASSERT_EQ(config.max_job, 128);
    TEST_ASSERT_EQ(config.debug, 1);
}

void test_config_load_missing_file(void)
{
    MiniShellConfig config;
    config_init(&config);
    int result = config_load(&config, "tests/not_exists.conf");
    TEST_ASSERT_EQ(result, MiniShell_ERR_OPEN);
}
