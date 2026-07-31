#ifndef SIG_H
#define SIG_H

#include "shell_context.h"
#include <signal.h>


void signal_init(ShellContext *ctx);
void signal_reset_child(void);

#endif
