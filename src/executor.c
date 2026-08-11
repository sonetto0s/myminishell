#include "executor.h"
#include "sig.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "error.h"
#include "log.h"

static void run_process(Command *com);

int execute_command(Command *com, ShellContext *ctx)
{
    if(!com->next)
        return execute_single(com,ctx);
    else
        return execute_pipeline(com);
}

int setredirect(Command *com)
{
    if (com->redirect.output_file)
    {
        int flags;
        if(com->redirect.append)
        {
            flags = O_WRONLY | O_CREAT | O_APPEND;
        }
        else
       {
          flags = O_WRONLY | O_CREAT | O_TRUNC;
       }
          int fd = open(com->redirect.output_file, flags, 0644);
          if (fd < 0)
          {
              log_error("open file failed: %s ", com->redirect.output_file);
              return MiniShell_ERR_OPEN;
          }
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            log_error("dup2 failed");
            return MiniShell_ERR_DUP2;
        }
        close(fd);
      
    }

    if (com->redirect.input_file)
    {
        int fd = open(com->redirect.input_file, O_RDONLY, 0644);
        if (fd < 0)
        {
            log_error("open file failed: %s ", com->redirect.input_file);
            return MiniShell_ERR_OPEN;
        }
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            log_error("dup2 failed");
            return MiniShell_ERR_DUP2;
        }
        close(fd);
    }
    return MiniShell_OK;
}

int execute_single(Command *com, ShellContext *ctx)
{
    int status;
    pid_t pid = fork();
    if (pid < 0)
    {
        log_error("fork failed");
        return MiniShell_ERR_FORK;
    }
    else if (pid == 0)
    {
        if (setredirect(com) != MiniShell_OK)
        {
            _exit(EXIT_FAILURE);
        }
        run_process(com);
    }
    if (com->background)
    {
        job_add(&ctx->jobs, pid, com->argv[0]);
        return MiniShell_OK;
    }

    if (waitpid(pid, &status, 0) < 0)
    {
        log_error("failed wait");
        return MiniShell_ERR_UNKNOWN;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }
    return MiniShell_ERR_UNKNOWN;
}

int execute_pipeline(Command *com)
{
    
    Command *current = com;
    int pipe_fd = -1;
    int count = 0;
    pid_t pids[64];
    int statuses = 0;
    int last_status = 0;
    while (current)
    {
        int pipefd[2];
        if (current->next)
        {
            if (pipe(pipefd) < 0)
            {
                log_error("pipe create failed");
                return MiniShell_ERR_PIPE;
            }
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            log_error("fork failed");
            return MiniShell_ERR_FORK;
        }
        if (pid > 0)
        {
            pids[count++] = pid;
        }

        if (pid == 0)
        {
            if (pipe_fd == -1 &&current->next != NULL)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                {
                    log_error("dup2 failed");
                    _exit(EXIT_FAILURE);
                }
                close(pipefd[1]);
                close(pipefd[0]);
                
            }
            else if (pipe_fd != -1 &&current->next != NULL)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                {
                    log_error("dup2 failed");
                    _exit(EXIT_FAILURE);
                }

                if (dup2(pipe_fd, STDIN_FILENO) == -1)
                {
                    log_error("dup2 failed");
                    _exit(EXIT_FAILURE);
                }
                close(pipefd[0]);
                close(pipefd[1]);
               
            }
            else if (current->next == NULL)
            {
                if (pipe_fd != -1)
                {
                    if (dup2(pipe_fd, STDIN_FILENO) == -1)
                    {
                        log_error("dup2 failed");
                        _exit(EXIT_FAILURE);
                    }
                    close(pipe_fd);
                }
               
            }
            if (setredirect(current) != MiniShell_OK)
            {
                _exit(EXIT_FAILURE);
            }
            run_process(current);
        }
        else
        {
            if (current->next)
            {
                if (pipe_fd != -1)
                {
                    close(pipe_fd);
                }
                pipe_fd = pipefd[0];
                close(pipefd[1]);
            }
            current = current->next;
        }
    }
    if (pipe_fd != -1)
    {
        close(pipe_fd);
    }
    for (int i = 0; i < count; i++)
    {
        if (waitpid(pids[i], &statuses, 0) < 0)
        {
            log_error("waitpid failed");
            return MiniShell_ERR_UNKNOWN;
        }
        if (i == count - 1)
            last_status = statuses;
    }
    if (WIFEXITED(last_status))
    {
        return WEXITSTATUS(last_status);
    }
    else if (WIFSIGNALED(last_status))
    {
        return 128 + WTERMSIG(last_status);
    }
    return MiniShell_ERR_UNKNOWN;
}

static void run_process(Command *com)
{
    signal_reset_child();
    execvp(com->argv[0], com->argv);
    log_error("execute command failed: %s", com->argv[0]);
    _exit(127);
}