#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

typedef enum
{
    JOB_RUNNING,
    JOB_DONE
} JobStatus;

typedef struct Job{
    int id;
    pid_t pid;
    JobStatus Status;
    char command[128];
    struct Job *next;
}Job;

typedef struct{
    Job *head;
    int nextid;
} JobManager;

void job_init(Job *job);
void jobmanager_init(JobManager *manager);
int job_add(JobManager *manager, pid_t pid, char *command);
void job_list(JobManager *manager);
Job *job_find(JobManager *manager, pid_t pid);
void job_remove(JobManager *manager, pid_t pids);
void job_reap(JobManager *manager);
void job_destroy(JobManager *manager);
#endif
