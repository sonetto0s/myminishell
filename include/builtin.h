#ifndef BUILTIN_H
#define BUILTIN_H

#include "command.h"
#include "shell_context.h"
typedef int (*BuiltinHandler)(Command *cmd,ShellContextStatus *ctx);
typedef struct{
    const char *name;
    BuiltinHandler handler;
} BuiltinEntry;

BuiltinEntry *builtin_lookup(const char *name);
int builtin_cd(Command *cmd, ShellContextStatus *ctx);
int builtin_pwd(Command *cmd, ShellContextStatus *ctx);
int builtin_exit(Command *cmd, ShellContextStatus *ctx);
#endif
