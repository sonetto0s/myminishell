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

void job_list(Job *head)
{
    Job *current = head;
    while(current)
    {
        printf("[%d] \n", current->id);
        printf("%s\n", current->command);
        if (current->Status == JOB_RUNNING)
            printf("now it is running\n");
        else
            printf("now it is done\n");

        current = current->next;
    }
}

Job *job_find(Job *head, pid_t pids)
{
    Job *current = head;
    while (current)
    {
        if (current->pid == pids)
            return current;

        current = current->next;
    }
    return NULL;
}

void job_remove(Job **head,pid_t pids)
{
    Job *current = *head;
    Job *prev = NULL;
    while (current)
    {
        if (current->pid == pids)
        {
            if (prev == NULL)
                *head = current->next;
            else
                prev->next = current->next;

            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}