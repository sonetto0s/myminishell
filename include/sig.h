#ifndef SIG_H
#define SIG_H

#define SIGNAL_EVENT_NONE      0
#define SIGNAL_EVENT_CHILD     (1 << 0)
#define SIGNAL_EVENT_INTERRUPT (1 << 1)

int signal_init(void);
int signal_take_events(void);

void signal_reset_child(void);

#endif

