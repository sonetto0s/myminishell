#include "shell_context.h"
#include "test_framework.h"
#include <string.h>

void test_shell_context_init(void)
{
    ShellContext ctx;
    memset(&ctx, 0xAA, sizeof(ctx));
    shell_context_init(&ctx);
    TEST_ASSERT_EQ(ctx.running, 1);
    TEST_ASSERT_EQ(ctx.last_exit_status, 0);
    TEST_ASSERT_NULL(ctx.jobs.head);
    TEST_ASSERT_EQ(ctx.jobs.nextid, 1);
    TEST_ASSERT_STR_EQ(ctx.config_file, "config/config.conf");
    shell_context_destroy(&ctx);
}

void test_shell_context_destroy(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    TEST_ASSERT_EQ(ctx.running, 1);
    shell_context_destroy(&ctx);
    TEST_ASSERT_EQ(ctx.running, 0);
    TEST_ASSERT_NULL(ctx.jobs.head);
}


