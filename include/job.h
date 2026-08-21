#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

typedef enum
{
    JOB_RUNNING,
    JOB_DONE
} JobStatus;

typedef struct Process{
    pid_t pid;
    int status;
    struct Process *next;
} Process;

typedef struct Job{
    int id;
    pid_t pgid;
    JobStatus Status;
    char command[128];
    Process *processes;
    struct Job *next;
}Job;

typedef struct{
    Job *head;
    int nextid;
} JobManager;

void job_init(Job *job);
void jobmanager_init(JobManager *manager);
Job *job_add(JobManager *manager, pid_t pgid, char *command);
void job_list(JobManager *manager);
Job *job_find(JobManager *manager, pid_t pgid);
void job_remove(JobManager *manager, pid_t pgid);
void job_reap(JobManager *manager);
void job_destroy(JobManager *manager);
void job_cleanup_done(JobManager *manager);
int process_add(Job *job, pid_t pid);

#endif
