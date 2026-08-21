#include "job.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

void job_init(Job *job)
{
    if (job == NULL)
        return;
    job->id = 0;
    job->pgid = 0;
    job->Status = JOB_RUNNING;
    memset(job->command, 0, sizeof(job->command));
    job->processes = NULL;
    job->next = NULL;
}

int job_add(JobManager *manager,pid_t pgid,char *command)
{
    Job *new_job = malloc(sizeof(Job));
    if (new_job == NULL)
    {
        log_error("malloc job failed");
        return -1;
    }
    job_init(new_job);
    new_job->id = manager->nextid++;
    new_job->pgid = pgid;
    strncpy(new_job->command, command, sizeof(new_job->command) - 1);
    new_job->command[sizeof(new_job->command) - 1] = '\0';
    new_job->next = manager->head;
    manager->head = new_job;
    return 0;
}

void job_list(JobManager *manager)
{
    Job *current = manager->head;
    while(current)
    {
        printf("[%d] %s\n", current->id, current->command);
        if (current->Status == JOB_RUNNING)
            printf("now it is running\n");
        else
            printf("now it is done\n");

        current = current->next;
    }
}

Job *job_find(JobManager *manager, pid_t pgid)
{
    Job *current = manager->head;
    while (current)
    {
        if (current->pgid == pgid)
            return current;

        current = current->next;
    }
    return NULL;
}

void job_remove(JobManager *manager, pid_t pgid)
{
    Job *current = manager->head;
    Job *prev = NULL;
    while (current)
    {
        if (current->pgid == pgid)
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
    Job *current = manager->head;
    int status;
    while (current)
    {
        Job *next = current->next;
        pid_t ret = waitpid(current->pgid, &status, WNOHANG);
        if (ret < 0)
        {
            if (errno != ECHILD)
            {
                log_error("waitpid failed");
            }
        }
        if (ret == current->pgid)
        {
            if (WIFEXITED(status))
            {
                printf("\n[%d] %s is finished,exit is%d\n", current->id, current->command, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("\n[%d] %s is killed by signal %d\n", current->id, current->command, WTERMSIG(status));
            }
            current->Status = JOB_DONE;
        }
        current = next;
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

void job_cleanup_done(JobManager *manager)
{
    Job *current = manager->head;
    Job *prev = NULL;
    while (current)
    {
        if (current->Status == JOB_DONE)
        {
            Job *temp = current;
            if (prev)
                prev->next = current->next;
            else
                manager->head = current->next;

            current = current->next;
            free(temp);
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }
}