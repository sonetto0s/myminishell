#ifndef ERROR_H
#define ERROR_H

typedef enum
{
    MiniShell_OK = 0,
    MiniShell_ERR_UNKNOWN = -1,
    MiniShell_ERR_MEMORY = -2,
    MiniShell_ERR_FORK = -3,
    MiniShell_ERR_PIPE = -4,
    MiniShell_ERR_EXEC = -5,
    MiniShell_ERR_OPEN = -6,
    MiniShell_ERR_PARSE = -7,
    MiniShell_ERR_JOB = -8,
    MiniShell_ERR_DUP2 = -9
} MiniShellError;

const char *minishell_error_string(MiniShellError error);
#endif
