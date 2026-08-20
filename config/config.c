#include "config.h"
#include "utils.h"
#include "error.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void config_init(MiniShellConfig * configs)
{
    if (configs == NULL)
        return;
    strncpy(configs->prompts, "MiniShell", sizeof(configs->prompts) - 1);
    configs->max_job = 64;
    configs->debug = 0;
}

int config_load(MiniShellConfig *configs, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        return MiniShell_ERR_OPEN;
    }
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
    trim_line(line);
    char *equal = strchr(line, '=');
    if (equal == NULL)
    {
        return;
    }
    *equal = '\0';
    char *key = line;
    char *value = equal + 1;
    if (strcmp(key, "prompts") == 0)
    {
        strncpy(configs->prompts, value, sizeof(configs->prompts) - 1);
        configs->prompts[sizeof(configs->prompts) - 1] = '\0';
    }
    else if (strcmp(key, "max_job") == 0)
    {
        configs->max_job = atoi(value);
    }
    else if (strcmp(key, "debug") == 0)
    {
        configs->debug = atoi(value);
    }
}