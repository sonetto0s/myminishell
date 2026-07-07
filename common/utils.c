#include "utils.h"


void trim_line(char *buff)
{
    int len = strlen(buff);
    if (len > 0 && buff[len-1]=='\n')
        buff[len - 1] = '\0';
}
