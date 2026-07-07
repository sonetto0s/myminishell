#include "utils.h"


// int is_exit(const Command*com)
// {
//     return (com->argc > 0 && strcmp(com->argv[0], "exit") == 0);
// }

void trim_line(char *buff)
{
    int len = strlen(buff);
    if (len > 0 && buff[len-1]=='\n')
        buff[len - 1] = '\0';
}
