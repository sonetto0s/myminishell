#ifndef BUILTIN_H
#define BUILTIN_H

#include "command.h"
#include "shell_context.h"
typedef int (*BuiltinHandler)(Command *cmd,ShellContext *ctx);
typedef struct{
    const char *name;
    BuiltinHandler handler;
} BuiltinEntry;

BuiltinEntry *builtin_lookup(const char *name);
int builtin_cd(Command *cmd, ShellContext *ctx);
int builtin_pwd(Command *cmd, ShellContext *ctx);
int builtin_exit(Command *cmd, ShellContext *ctx);
int builtin_job(Command *cmd, ShellContext *ctx);
#endif
