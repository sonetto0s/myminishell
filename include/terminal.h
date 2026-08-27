#ifndef TERMINAL_H
#define TERMINAL_H
#include <unistd.h>

int terminal_init(void);
int terminal_get_fd(void);
int terminal_set_foreground(pid_t pgid);
int terminal_restore(void);

#endif
