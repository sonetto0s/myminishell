#include "executor.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int execute_command(Command *com)
{
    if(!com->next)
        execute_single(com);
    else
        execute_pipeline(com);
   
    return 0;
}

int execute_single(Command *com)
{
    pid_t pid = fork();
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
        execvp(com->argv[0], com->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    waitpid(pid, NULL, 0);
    return 0;
}

int execute_pipeline(Command *com)
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        return -1;
    }
    Command *cmd1 = com;
    Command *cmd2 = com->next;
    pid_t pid1 = fork();
    
    if (pid1 < 0)
    {
        perror("fork");
        return -1;
    }
    else if (pid1 == 0)
    {
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            return -1;
        }
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1->argv[0], cmd1->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        perror("fork");
        return -1;
    }
    else if (pid2 == 0)
    {
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("dup2");
            return -1;
        }
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd2->argv[0], cmd2->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}