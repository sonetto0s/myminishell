#include "error.h"

const char *minishell_error_string(MiniShellError error)
{
    switch (error) {
    case MiniShell_OK:
        return "success";

    case MiniShell_ERR_UNKNOWN:
        return "unknown error";

    case MiniShell_ERR_MEMORY:
        return "memory allocation failed";

    case MiniShell_ERR_FORK:
        return "fork failed";

    case MiniShell_ERR_PIPE:
        return "pipe operation failed";

    case MiniShell_ERR_EXEC:
        return "command execution failed";

    case MiniShell_ERR_OPEN:
        return "file open failed";

    case MiniShell_ERR_PARSE:
        return "parse failed";

    case MiniShell_ERR_JOB:
        return "job operation failed";

    case MiniShell_ERR_DUP2:
        return "dup2 failed";

    default:
        return "unidentified error";
    }
}
