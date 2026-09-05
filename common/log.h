#ifndef LOG_H
#define LOG_H

#if defined(__GNUC__) || defined(__clang__)
#define MINISHELL_PRINTF_FORMAT(format_index, first_arg) \
    __attribute__((format(printf, format_index, first_arg)))
#else
#define MINISHELL_PRINTF_FORMAT(format_index, first_arg)
#endif

typedef enum
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_ERR
} Loglevel;

void log_init(void);
void log_setlevel(Loglevel level);

void log_debug(const char *format, ...)
    MINISHELL_PRINTF_FORMAT(1, 2);

void log_error(const char *format, ...)
    MINISHELL_PRINTF_FORMAT(1, 2);

void log_info(const char *format, ...)
    MINISHELL_PRINTF_FORMAT(1, 2);

#endif
