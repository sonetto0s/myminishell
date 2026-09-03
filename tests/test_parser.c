#include <string.h>
#include "parser.h"
#include "shell_context.h"
#include "command.h"
#include "test_framework.h"

void test_parser(void)
{
    ShellContext ctx;
    shell_context_init(&ctx);
    char input[] = "ls -l";
    Command *cmd = parse_line(input, &ctx);
   TEST_ASSERT_NOT_NULL(cmd);
   if (cmd != NULL)
   {
       TEST_ASSERT_STR_EQ(cmd->argv[0], "ls");
       TEST_ASSERT_STR_EQ(cmd->argv[1], "-l");
   }
    command_free(cmd);
    shell_context_destroy(&ctx);
}
