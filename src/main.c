#include "shell.h"
#include "stdio.h"
int main()
{
    ShellContext ctx;
    if (shell_init(&ctx) != SHELL_STATUS_OK)
    {
        return -1;
    }
    shell_run(&ctx);
    shell_cleanup(&ctx);

    return 0;
}
