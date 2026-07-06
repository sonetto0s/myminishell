#include "shell.h"
#include "utils.h"
#include <stdio.h>
ShellStatus shell_init(void)
{
    printf(">>shell初始化成功\r\n");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(void)
{   
    while(1)
  {
    printf(">>MiniShell\r\n");
    char buff[100];
    char *fgetsresult = fgets(buff, sizeof(buff), stdin);
    if (fgetsresult == NULL)
        break;

    trim_line(fgetsresult);
    if (is_exit(fgetsresult))
    {
      printf(">>MiniShell程序已结束\r\n");
      break;
    }
    // else
    // {
    //    printf(">>MiniShell程序关闭失败\r\n");
    //    return SHELL_STATUS_ERROR;
    //  }
    
  }
  return SHELL_STATUS_OK;
}
void shell_cleanup(void)
{
    printf(">>MiniShell 已退出\r\n");
}