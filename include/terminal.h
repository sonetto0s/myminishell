#ifndef TERMINAL_H
#define TERMINAL_H

#include <sys/types.h>
#include <termios.h>

int terminal_init(void);
void terminal_shutdown(void);
int terminal_is_initialized(void);
int terminal_set_foreground(pid_t pgid);
int terminal_get_modes(struct termios *modes);
int terminal_set_modes(const struct termios *modes);
int terminal_restore(void);

#endif


