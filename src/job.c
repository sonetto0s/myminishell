#include "job.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

static void process_destroy(Process *process)
{
    while (process)
    {
        Process *next = process->next;
        free(process);
        process = next;
    }
}

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

Job *job_add(JobManager *manager, pid_t pgid, char *command)
{
    Job *new_job = malloc(sizeof(Job));
    if (new_job == NULL)
    {
        log_error("malloc job failed");
        return NULL;
    }
    job_init(new_job);
    new_job->id = manager->nextid++;
    new_job->pgid = pgid;
    strncpy(new_job->command, command, sizeof(new_job->command) - 1);
    new_job->command[sizeof(new_job->command) - 1] = '\0';
    new_job->next = manager->head;
    manager->head = new_job;
    return new_job;
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

            process_destroy(current->processes);
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
    Job *job = manager->head;
    while (job)
    {
        Process *process = job->processes;
        int done = 1;
        while (process)
        {
            int status;
            pid_t ret = waitpid(process->pid, &status, WNOHANG);
            if (ret < 0)
            {
                if (errno == ECHILD)
                {
                    process->status = 1;
                }
                else
                {
                    log_error("failed waitpid");
                }
            }
            else if (ret == process->pid)
            {
                process->status = 1;
                if (WIFEXITED(status))
                {
                    printf("\n[%d] process %d exit %d\n", job->id, process->pid, WEXITSTATUS(status));
                }
                else if (WIFSIGNALED(status))
                {
                    printf("\n[%d] process %d killed by %d\n", job->id, process->pid, WTERMSIG(status));
                }
            }
            if (process->status == 0)
            {
                done = 0;
            }
            process = process->next;
        }
        if (done)
        {
            job->Status = JOB_DONE;
        }
        job = job->next;
    }
}

void job_destroy(JobManager *manager)
{
    Job *current = manager->head;
    while (current)
    {
        Job *next = current->next;
        process_destroy(current->processes);
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
            process_destroy(temp->processes);
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

int process_add(Job *job, pid_t pid)
{
    Process *new_process = malloc(sizeof(Process));
    if (new_process == NULL)
    {
        log_error("failed malloc process");
        return -1;
    }
    new_process->pid = pid;
    new_process->status = 0;
    new_process->next = NULL;
    if (job->processes == NULL)
    {
        job->processes = new_process;
    }
    else
    {
        Process *current = job->processes;
        while (current->next)
        {
            current = current->next;
        }
        current->next = new_process;
    }
    return 0;
}

