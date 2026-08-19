#include "job.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

void job_init(Job *job)
{
    if (job == NULL)
        return;
    job->id = 0;
    job->pid = 0;
    job->Status = JOB_RUNNING;
    memset(job->command, 0, sizeof(job->command));
    job->next = NULL;
}

void job_add(JobManager *manager,pid_t pid,char *command)
{
    Job *new_job = malloc(sizeof(Job));
    if (new_job == NULL)
    {
        log_error("malloc job failed");
        return;
    }
    job_init(new_job);
    new_job->id = manager->nextid++;
    new_job->pid = pid;
    strncpy(new_job->command, command, sizeof(new_job->command) - 1);
    new_job->command[sizeof(new_job->command) - 1] = '\0';
    new_job->next = manager->head;
    manager->head = new_job;

}

void job_list(JobManager *manager)
{
    Job *current = manager->head;
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

Job *job_find(JobManager *manager, pid_t pids)
{
    Job *current = manager->head;
    while (current)
    {
        if (current->pid == pids)
            return current;

        current = current->next;
    }
    return NULL;
}

void job_remove(JobManager *manager, pid_t pids)
{
    Job *current = manager->head;
    Job *prev = NULL;
    while (current)
    {
        if (current->pid == pids)
        {
            if (prev == NULL)
                manager->head = current->next;
            else
                prev->next = current->next;

            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}
 
void jobmanager_init(JobManager *manager)
{
    manager->head = NULL;
    manager->nextid = 1;
}

void job_reap(JobManager *manager)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        Job *job = job_find(manager, pid);
        if (job)
        {
            printf("\n[%d] %s is finished\n", job->id, job->command);
        }
        job_remove(manager, pid);
    }
}

void job_destroy(JobManager *manager)
{
    Job *current = manager->head;
    while (current)
    {
        Job *next = current->next;
        free(current);
        current = next;
    }
    manager->head = NULL;
    manager->nextid = 1;
}