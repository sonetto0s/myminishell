#ifndef CONFIG_H
#define CONFIG_H

typedef struct{
    char prompts[64];
    int max_job;
    int debug;
} MiniShellConfig;

void config_init(MiniShellConfig *configs);
int config_load(MiniShellConfig *configs,const char *filename);
void config_parse_line(char *line, MiniShellConfig *configs);

#endif



