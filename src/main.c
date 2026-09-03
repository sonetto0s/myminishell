#include "shell.h"

int main(void)
{
    ShellContext ctx;
    if (shell_init(&ctx) != SHELL_STATUS_OK)
    {
        return 1;
    }
    ShellStatus run_status = shell_run(&ctx);
    int exit_status = ctx.last_exit_status;
    shell_cleanup(&ctx);
    if (run_status != SHELL_STATUS_OK)
    {
        return 1;
    }
    return exit_status;
}
