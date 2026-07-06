#include "utils.h"
#include <stdio.h>
#include <string.h>

int is_exit(const char *buff)
{
    return (strcmp(buff, "exit") == 0);
}

void trim_line(char *buff)
{
    int len = strlen(buff);
    if (len > 0 && buff[len-1]=='\n')
        buff[len - 1] = '\0';
}
