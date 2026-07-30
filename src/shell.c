#include "shell.h"
#include "utils.h"
#include "parser.h"
#include <stdio.h>
#include <errno.h>
#include "dispatcher.h"
#include "executor.h"
#include "command.h"
#include "sig.h"

static ShellContext ctx;

ShellStatus shell_init(void)
{
    shell_context_init(&ctx);
    signal_init(&ctx);
    printf(">>shell初始化成功\r\n");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(void)
{
    while (ctx.running)
    {
        Command *cmd_list = NULL;
        printf(">>MiniShell\r\n");
        char buff[100];
        char *fgetsresult = fgets(buff, sizeof(buff), stdin);
        if (fgetsresult == NULL)
        {
            if (feof(stdin))
            {
                break;
            }
            if (errno == EINTR)
            {
                clearerr(stdin);
                continue;
            }
            break;
        }

        trim_line(fgetsresult);
        cmd_list=parse_line(fgetsresult,&ctx);
        if (cmd_list != NULL)
        {
            int status = dispatcher_command(cmd_list,&ctx);
            ctx.last_exit_status = status;
        }
        else
        {
           continue;
        }
        command_free(cmd_list);
    }
  return SHELL_STATUS_OK;
}
void shell_cleanup(void)
{
    printf(">>MiniShell 已退出\r\n");
}