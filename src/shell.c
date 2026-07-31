#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include "shell.h"
#include "utils.h"
#include "parser.h"
#include "dispatcher.h"
#include "executor.h"
#include "command.h"
#include "sig.h"
#include "event.h"
#include "job.h"

static ShellContext ctx;

ShellStatus shell_init(void)
{
    shell_context_init(&ctx);
    if (event_init() < 0)
    {
        return SHELL_STATUS_ERROR;
    }
    signal_init(&ctx);
    printf(">>shell初始化成功\r\n");
    return SHELL_STATUS_OK;
}

ShellStatus shell_run(void)
{
    int event_fd = event_getfd();
    while (ctx.running)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO,&readfds);
        FD_SET(event_fd,&readfds);
        int max_fd = event_fd > STDIN_FILENO ? event_fd : STDIN_FILENO;
        int ret = select(max_fd+1,&readfds,NULL,NULL,NULL);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }
        if (FD_ISSET(event_fd, &readfds))
        {
            char buffer[8];
            read(event_fd, buffer, sizeof(buffer));
            job_reap(&ctx.jobs);
        }
        if (FD_ISSET(STDIN_FILENO, &readfds))
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
            cmd_list = parse_line(fgetsresult, &ctx);
            if (cmd_list != NULL)
            {
                int status = dispatcher_command(cmd_list, &ctx);
                ctx.last_exit_status = status;
            }
            else
            {
                continue;
            }
            command_free(cmd_list);
        }
        
    }
  return SHELL_STATUS_OK;
}
void shell_cleanup(void)
{
    event_shut();
    printf(">>MiniShell 已退出\r\n");
}