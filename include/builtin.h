#ifndef BUILTIN_H
#define BUILTIN_H

#include "command.h"

typedef int (*BuiltinHandler)(Command *cmd);
typedef struct{
    const char *name;
    BuiltinHandler handler;
} BuiltinEntry;

BuiltinEntry *builtin_lookup(const char *name);
int builtin_cd(Command *cmd);
int builtin_pwd(Command *cmd);
int builtin_exit(Command *cmd);
#endif
