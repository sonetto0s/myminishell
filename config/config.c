#include "config.h"
#include "utils.h"
#include "error.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void config_init(MiniShellConfig *configs)
{
    if (!configs)
        return;
    strncpy(configs->prompts, "MiniShell", sizeof(configs->prompts) - 1);
    configs->prompts[sizeof(configs->prompts) - 1] = '\0';
    configs->max_job = 64;
    configs->debug = 0;
}

int config_load(MiniShellConfig *configs, const char *filename)
{
    if (!configs || !filename)
        return MiniShell_ERR_UNKNOWN;

    FILE *fp = fopen(filename, "r");
    if (!fp)
        return MiniShell_ERR_OPEN;

    char line[128];

    while (fgets(line, sizeof(line), fp))
    {
        config_parse_line(line, configs);
    }

    fclose(fp);
    return MiniShell_OK;
}

void config_parse_line(char *line, MiniShellConfig *configs)
{
    if (!line || !configs) return;

    trim_line(line);

    char *equal = strchr(line, '=');
    if (!equal) return;

    *equal = '\0';

    char *key = line;
    char *value = equal + 1;

    if (strcmp(key, "prompts") == 0)
    {
        strncpy(configs->prompts, value, sizeof(configs->prompts) - 1);
        configs->prompts[sizeof(configs->prompts) - 1] = '\0';
        return;
    }

    if (strcmp(key, "max_job") == 0)
    {
        errno = 0;

        char *end = NULL;
        long number = strtol(value, &end, 10);

        if (errno == 0 &&end != value &&*end == '\0' &&number > 0 &&number <= INT_MAX)
        {
            configs->max_job = (int)number;
        }

        return;
    }

    if (strcmp(key, "debug") == 0)
    {
        if (strcmp(value, "0") == 0)
            configs->debug = 0;
        else if (strcmp(value, "1") == 0)
            configs->debug = 1;
    }
}





