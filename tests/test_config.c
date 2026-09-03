#include "config.h"
#include "test_framework.h"

void test_config(void)
{
    MiniShellConfig config;
    config_init(&config);
    TEST_ASSERT_EQ(config.debug, 0);
}

