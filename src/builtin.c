#include "builtin.h"

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
    return 0;
}
int builtin_pwd(Command *cmd)
{
    return 0;
}

int builtin_exit(Command *cmd)
{
    return 0;
}
