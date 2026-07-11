#include "shell.h"
#include "utils.h"
#include "parser.h"
#include <stdio.h>
#include "dispatcher.h"
#include "executor.h"
#include "command.h"
ShellStatus shell_init(void)
{
    printf(">>shell初始化成功\r\n");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(void)
{
    Command *cmd_list = NULL;
    while (1)
    {
        printf(">>MiniShell\r\n");
        char buff[100];
        char *fgetsresult = fgets(buff, sizeof(buff), stdin);
        if (fgetsresult == NULL)
            break;
        trim_line(fgetsresult);
        cmd_list=parse_line(fgetsresult);
        if (cmd_list != NULL)
            dispatcher_command(cmd_list);
        else
        {
           printf(">>MiniShell程序关闭失败\r\n");
           return SHELL_STATUS_ERROR;
        }
    }
  return SHELL_STATUS_OK;
}
void shell_cleanup(void)
{
    printf(">>MiniShell 已退出\r\n");
}