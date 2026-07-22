#include "builtin.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static BuiltinEntry builtin_table[] = {
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"exit", builtin_exit},
};

BuiltinEntry *builtin_lookup(const char *name)
{
    for (int i = 0; i < sizeof(builtin_table) / sizeof(builtin_table[0]); i++)
    {
        if (strcmp(name, builtin_table[i].name) == 0)
            return &builtin_table[i];
    }
    return NULL;
 
}

int builtin_cd(Command * cmd)
{
    if (cmd->argc <2)
     {
         fprintf(stderr,"你cd后面没写东西\r\n");
         return -1;
     }
      
    if (chdir(cmd->argv[1]) == -1)
    {
        perror("chdir");
        return -1;
    }
    
    return 0;
}
int builtin_pwd(Command *cmd)
{
    char buff[100];
    if (getcwd(buff, sizeof(buff)) != NULL)
    {
        fprintf(stdout,"%s\n",buff);
        return 0;
    }
    else
    {
        perror("getcwd");
        return -1;
    }
}

int builtin_exit(Command *cmd)
{
    exit(0);
    return 0;
}
