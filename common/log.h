#ifndef LOG_H
#define LOG_H

typedef enum
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERR
} Loglevel;

void log_init(void);
void log_setlevel(Loglevel level);
void log_debug(const char *format,...);
void log_error(const char *format, ...);
void log_info(const char *format, ...);
#endif
