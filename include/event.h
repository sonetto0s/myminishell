#ifndef EVENT_H
#define EVENT_H


int event_init(void);
int event_getfd(void);
void event_notify(void);
void event_shut(void);
void event_close_in_child(void);

#endif
