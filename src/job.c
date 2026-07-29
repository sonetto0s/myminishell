#include "job.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void job_init(Job *job)
{
    job->id = 0;
    job->pid = 0;
    job->Status = JOB_RUNNING;
    memset(job->command, 0, sizeof(job->command));
    job->next = NULL;
}

void job_add(Job **head,pid_t pid,char *command,int id)
{
    Job *new_job = malloc(sizeof(Job));
    if (new_job == NULL)
    {
        perror("malloc");
        return;
    }
    job_init(new_job);
    new_job->id = id;
    new_job->pid = pid;
    strncpy(new_job->command, command, sizeof(new_job->command) - 1);
    new_job->command[sizeof(new_job->command) - 1] = '\0';
    new_job->next = *head;
    *head = new_job;
}
