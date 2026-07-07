#include "executor.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
int execute_command(Command *com)
{
    // printf("argc=%d\n", com->argc);

    // for (int i = 0; i < com->argc; i++)
    // {
    //     printf("argv[%d]=%s\n", i, com->argv[i]);
    // }

    pid_t pid=fork();
    if (pid < 0)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0)
    {
        if (com->redirect.output_file != NULL)
        {
            int fd = open(com->redirect.output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        if (com->redirect.input_file != NULL)
        {
            int fd = open(com->redirect.input_file, O_RDONLY, 0644);
            if (fd < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            if (dup2(fd, STDIN_FILENO) == -1)
            {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        execvp(com->argv[0],com->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    waitpid(pid,NULL,0);
    return 0;
}
