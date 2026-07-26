#ifndef SHELL_CONTEXT_H
#define SHELL_CONTEXT_H

typedef struct 
{
    int last_exit_status;
} ShellContextStatus;

void shell_context_init(ShellContextStatus *ctx);

#endif
