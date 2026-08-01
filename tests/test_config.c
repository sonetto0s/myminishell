#include "config.h"
#include <stdio.h>

void test_config()
{
    MiniShellConfig config;
    config_init(&config);
    if (config.debug == 0)
    {
        printf("[pass] Config init success\n");
    }
    else
    {
        printf("[fail] Config init failed\n");
    }
}
