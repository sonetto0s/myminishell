#include "shell.h"
#include "job.h"
#include "stdio.h"
int main()
{
    ShellContext ctx;
    shell_init();
    shell_run();
    shell_cleanup();

    return 0;
}
