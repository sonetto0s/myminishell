#include "log.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>

static Loglevel current_level;

void log_init(void)
{
    current_level = LOG_DEBUG;
}

void log_setlevel(Loglevel level)
{
    current_level = level;
}

static void log_write(Loglevel level, const char *format, va_list args)
{
    if (level < current_level) return;
    switch (level)
    {
    case LOG_DEBUG:
        fprintf(stderr, "[DEBUG] ");
        break;
        case LOG_INFO:
            fprintf(stderr, "[INFO] ");
            break;
    case LOG_ERR:
        fprintf(stderr, "[ERROR] ");
        break;
    default:
        fprintf(stderr, "[UNKNOWN] ");
        break;
    }
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

void log_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(LOG_DEBUG, format, args);
    va_end(args);
}


void log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_write(LOG_ERR, format, args);
    va_end(args);
}


void log_info(const char *format, ...)
{
    va_list args;
    va_start(args,format);
    log_write(LOG_INFO, format, args);
    va_end(args);
}
