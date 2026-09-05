#include "config.h"
#include "error.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim_space(char *text)
{
    while (*text && isspace((unsigned char)*text))
        text++;

    char *end = text + strlen(text);

    while (end > text && isspace((unsigned char)end[-1]))
        end--;

    *end = '\0';

    return text;
}

void config_init(MiniShellConfig *configs)
{
    if (!configs) return;

    strncpy(configs->prompts, "MiniShell", sizeof(configs->prompts) - 1);
    configs->prompts[sizeof(configs->prompts) - 1] = '\0';

    configs->max_job = 64;
    configs->debug = 0;
}

int config_parse_line(char *line, MiniShellConfig *configs)
{
    if (!line || !configs)
        return MiniShell_ERR_UNKNOWN;

    char *text = trim_space(line);

    if (*text == '\0' || *text == '#')
        return MiniShell_OK;

    char *equal = strchr(text, '=');

    if (!equal)
        return MiniShell_ERR_PARSE;

    *equal = '\0';

    char *key = trim_space(text);
    char *value = trim_space(equal + 1);

    if (*key == '\0')
        return MiniShell_ERR_PARSE;

    if (strcmp(key, "prompts") == 0) {
        if (*value == '\0' || strlen(value) >= sizeof(configs->prompts))
            return MiniShell_ERR_PARSE;

        strcpy(configs->prompts, value);
        return MiniShell_OK;
    }

    if (strcmp(key, "max_job") == 0) {
        errno = 0;

        char *end = NULL;
        long number = strtol(value, &end, 10);

        if (errno != 0 ||
            end == value ||
            *end != '\0' ||
            number <= 0 ||
            number > INT_MAX)
            return MiniShell_ERR_PARSE;

        configs->max_job = (int)number;
        return MiniShell_OK;
    }

    if (strcmp(key, "debug") == 0) {
        if (strcmp(value, "0") == 0) {
            configs->debug = 0;
            return MiniShell_OK;
        }

        if (strcmp(value, "1") == 0) {
            configs->debug = 1;
            return MiniShell_OK;
        }

        return MiniShell_ERR_PARSE;
    }

    return MiniShell_ERR_PARSE;
}

int config_load(MiniShellConfig *configs, const char *filename)
{
    if (!configs || !filename)
        return MiniShell_ERR_UNKNOWN;

    FILE *fp = fopen(filename, "r");

    if (!fp) {
        fprintf(stderr, "config: open '%s' failed: %s\n",
                filename, strerror(errno));

        return MiniShell_ERR_OPEN;
    }

    MiniShellConfig temporary;
    config_init(&temporary);

    char line[256];
    int result = MiniShell_OK;
    int line_number = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_number++;

        size_t length = strlen(line);

        if (length > 0 &&
            line[length - 1] != '\n' &&
            !feof(fp)) {
            int ch;

            while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
            }

            fprintf(stderr,
                    "config: line %d is too long\n",
                    line_number);

            result = MiniShell_ERR_PARSE;
            break;
        }

        result = config_parse_line(line, &temporary);

        if (result != MiniShell_OK) {
            fprintf(stderr,
                    "config: invalid line %d\n",
                    line_number);

            break;
        }
    }

 if (result == MiniShell_OK && ferror(fp)) {
    int saved_errno = errno ? errno : EIO;

        fprintf(stderr,
                "config: read '%s' failed: %s\n",
                filename,
                strerror(saved_errno));

        result = MiniShell_ERR_OPEN;
    }

    if (fclose(fp) != 0 && result == MiniShell_OK) {
        fprintf(stderr,
                "config: close '%s' failed: %s\n",
                filename,
                strerror(errno));

        result = MiniShell_ERR_OPEN;
    }

    if (result == MiniShell_OK)
        *configs = temporary;

    return result;
}


