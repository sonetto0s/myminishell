#define _GNU_SOURCE
#include "system_info.h"
#include "log.h"
#include "error.h"
#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int get_cpu_info(SystemInfo *info);
static int get_mem_info(SystemInfo *info);
static int get_uptime_info(SystemInfo *info);

int system_info_collect(SystemInfo *info)
{
    if (info == NULL)
    {
        log_error("system_info is     ");
        return MiniShell_ERR_UNKNOWN;
    }

    memset(info, 0, sizeof(SystemInfo));

    struct utsname uts;
    if (uname(&uts) < 0)
    {
        log_error("failed uname");
        return MiniShell_ERR_UNKNOWN;
    }
    strncpy(info->kernel, uts.release, sizeof(info->kernel) - 1);
    info->kernel[sizeof(info->kernel) - 1] = '\0';
    strncpy(info->architecture, uts.machine, sizeof(info->architecture) - 1);
    info->architecture[sizeof(info->architecture) - 1] = '\0';
    if (gethostname(info->hostname, sizeof(info->hostname) - 1) < 0)
    {
        log_error("gethostname is failed");
        return MiniShell_ERR_UNKNOWN;
    }
    info->hostname[sizeof(info->hostname) - 1] = '\0';
    int ret = get_cpu_info(info);
    if (ret != MiniShell_OK)
    {
        return ret;
    }
    ret = get_mem_info(info);
    if (ret != MiniShell_OK)
    {
        return ret;
    }
    ret = get_uptime_info(info);
    if (ret != MiniShell_OK)
    {
        return ret;
    }
    return MiniShell_OK;
}

void system_info_print(SystemInfo *info)
{
    if (info == NULL)
    {
        log_error("system_info is     ");
        return;
    }
    printf("\n");
    printf("========== System ==========\n");

    printf("Kernel       : %s\n", info->kernel);
    printf("Hostname     : %s\n", info->hostname);
    printf("Architecture : %s\n", info->architecture);
    printf("CPU Model : %s\n", info->cpu_model);
    printf("Memory Total : %lu MB\n", info->mem_total);
    printf("Memory Available : %lu MB\n", info->mem_available);
    printf("Uptime : %.2f seconds\n", info->uptime);
    printf("\n");
}

static int get_cpu_info(SystemInfo *info)
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL)
    {
        log_error("open /proc/cpuinfo failed");
        return MiniShell_ERR_OPEN;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "model name", 10) == 0)
        {
            char *colon = strchr(line, ':');
            if (colon)
            {
                colon++;
                while (*colon == ' ' || *colon == '\t')
                    colon++;
                strncpy(info->cpu_model, colon, sizeof(info->cpu_model) - 1);
                info->cpu_model[strcspn(info->cpu_model, "\n")] = '\0';
            }
            break;
        }
    }
    fclose(fp);
    return MiniShell_OK;
}

static int get_mem_info(SystemInfo *info)
{
    FILE *fp = fopen("/proc/meminfo","r");
    if (fp == NULL)
    {
        log_error("open /proc/meminfo failed");
        return MiniShell_ERR_OPEN;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "MemTotal:", 9) == 0)
        {
            if (sscanf(line, "MemTotal:%lu", &info->mem_total) != 1)
            {
                log_error("MemTotal failed");
            }
        }
        else if (strncmp(line, "MemAvailable:", 13) == 0)
        {
            if (sscanf(line, "MemAvailable:%lu", &info->mem_available) != 1)
            {
                log_error("MemAvailable failed");
            }
        }
    }
    fclose(fp);
    info->mem_total /= 1024;
    info->mem_available /= 1024;
    return MiniShell_OK;
}

static int get_uptime_info(SystemInfo *info)
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp == NULL)
    {
        log_error("open /proc/uptime failed");
        return MiniShell_ERR_OPEN;
    }
    if (fscanf(fp, "%lf", &info->uptime) != 1)
    {
        log_error("uptime failed");
        fclose(fp);
        return MiniShell_ERR_UNKNOWN;
    }

    fclose(fp);
    return MiniShell_OK;
}