#include "executor.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "stdlib.h"
int execute_execute(Command *com)
{
    pid_t pid=fork();
    if (pid < 0)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0)
    {   
        execvp(com->argv[0],com->argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    waitpid(pid,NULL,0);
    return 0;
}
