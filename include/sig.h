#ifndef SIG_H
#define SIG_H

#include "shell_context.h"

void signal_init(ShellContextStatus *ctx);
void signal_reset_child(void);

#endif
