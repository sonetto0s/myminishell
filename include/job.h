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

void job_init(Job *job);
void job_add(Job **head, pid_t pid, char *command, int id);

#endif
