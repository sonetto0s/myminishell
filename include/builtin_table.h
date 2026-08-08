#ifndef BUILTIN_TABLE_H
#define BUILTIN_TABLE_H

#include "command.h"
#include <stddef.h>

struct ShellContext;

typedef int (*Builtinhandler)(Command *cmd, struct ShellContext *ctx);
typedef struct
{
    const char *name;
    Builtinhandler handler;
} BuiltinEntry;

BuiltinEntry *builtin_lookup(const char *name);
BuiltinEntry *builtin_get(size_t index);
size_t builtin_count(void);

#endif
