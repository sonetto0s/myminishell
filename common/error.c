#include "error.h"

const char *Minishellerror_string(MiniShellError error)
{
    switch(error)
    {
        case MiniShell_OK:
            return "success";
        case MiniShell_ERR_FORK:
            return "Failed fork operation";
        case MiniShell_ERR_EXEC:
            return "Failed execute operation";
        case MiniShell_ERR_JOB:
            return "Failed job operation";
        case MiniShell_ERR_MEMORY:
            return "Failed memory operation";
        case MiniShell_ERR_OPEN:
            return "Failed open operation";
        case MiniShell_ERR_PARSE:
            return "Failed parse operation";
        case MiniShell_ERR_PIPE:
            return "Failed pipe operation";
        case MiniShell_ERR_UNKNOWN:
            return "Failed unknown operation";
        case MiniShell_ERR_DUP2:
            return "Failed dup2 operation";
        default:
            return "strange && undentified error🤔";
    }
}
