#include "job.h"
#include "log.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <termios.h>

#define JOB_SHUTDOWN_RETRY_COUNT 50
#define JOB_SHUTDOWN_WAIT_NS 10000000L

static void process_destroy(Process *process)
{
    while (process)
    {
        Process *next = process->next;
        free(process);
        process = next;
    }
}

static Process *process_find(Job *job, pid_t pid)
{
    if (!job || pid <= 0)
        return NULL;

    Process *process = job->processes;

    while (process)
    {
        if (process->pid == pid)
            return process;
        process = process->next;
    }
    return NULL;
}

static void job_update_status(Job *job)
{
    if (!job || !job->processes)
        return;

    int has_running = 0;
    int has_stopped = 0;
    Process *process = job->processes;

    while (process)
    {
        if (process->status == PROCESS_RUNNING)
            has_running = 1;
        else if (process->status == PROCESS_STOPPED)
            has_stopped = 1;
        process = process->next;
    }

    if (has_running) job->status = JOB_RUNNING;
    else if (has_stopped) job->status = JOB_STOPPED;
    else job->status = JOB_DONE;
}

static void process_apply_wait_status(Process *process, int status)
{
    if (!process) return;

    process->wait_status = status;
    process->wait_status_valid = 1;

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        process->status = PROCESS_DONE;
    } else if (WIFSTOPPED(status)) {
        process->status = PROCESS_STOPPED;
    } else if (WIFCONTINUED(status)) {
        process->status = PROCESS_RUNNING;
    }
}

// static void job_apply_wait_status(Job *job, pid_t pid, int status)
// {
//     Process *process = process_find(job, pid);
//     if (!process) return;

//     process_apply_wait_status(process, status);
//     job_update_status(job);
// }

static Process *job_last_process(const Job *job)
{
    if (!job || !job->processes) return NULL;

    Process *process = job->processes;

    while (process->next) process = process->next;

    return process;
}

static void shutdown_sleep(void)
{
    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = JOB_SHUTDOWN_WAIT_NS
    };

    while (nanosleep(&ts, &ts) < 0) {
        if (errno != EINTR) break;
    }
}

static int process_try_reap(Process *process)
{
    if (!process || process->status == PROCESS_DONE) return 1;

    int status = 0;

    while (1) {
        pid_t ret = waitpid(process->pid, &status, WNOHANG);

        if (ret == process->pid) {
            process_apply_wait_status(process, status);
            return 1;
        }

        if (ret == 0) return 0;
        if (errno == EINTR) continue;

        if (errno == ECHILD) {
            process->status = PROCESS_DONE;
            return 1;
        }

        log_error("failed reap process %d during shutdown", process->pid);
        return 0;
    }
}

static void process_force_reap(Process *process)
{
    if (!process || process->status == PROCESS_DONE) return;

    while (1) {
        int status = 0;
        pid_t ret = waitpid(process->pid, &status, 0);

        if (ret == process->pid) {
            process_apply_wait_status(process, status);
            return;
        }

        if (ret < 0 && errno == EINTR) continue;

        if (ret < 0 && errno == ECHILD) {
            process->status = PROCESS_DONE;
            return;
        }

        if (ret < 0) log_error("failed wait process %d during shutdown", process->pid);
        return;
    }
}

static int jobmanager_try_reap_all(JobManager *manager)
{
    if (!manager) return 1;

    int all_done = 1;
    Job *job = manager->head;

    while (job) {
        Process *process = job->processes;

        while (process) {
            if (!process_try_reap(process)) all_done = 0;
            process = process->next;
        }

        job_update_status(job);
        job = job->next;
    }

    return all_done;
}

// static void signal_job_processes(Job *job, int sig)
// {
//     if (!job) return;

//     Process *process = job->processes;

//     while (process) {
//         if (process->status != PROCESS_DONE) {
//             if (kill(process->pid, sig) < 0 && errno != ESRCH) {
//                 log_error("failed send signal %d to process %d", sig, process->pid);
//             }
//         }

//         process = process->next;
//     }
// }

static void continue_stopped_job(Job *job)
{
    if (!job) return;

    if (job->pgid > 0) {
        if (kill(-job->pgid, SIGCONT) < 0 && errno != ESRCH) {
            log_error("failed continue job %d during shutdown", job->id);
        }
    }

    Process *process = job->processes;

    while (process) {
        if (process->status == PROCESS_STOPPED) process->status = PROCESS_RUNNING;
        process = process->next;
    }

    job_update_status(job);
}
void job_init(Job *job)
{
    if (!job) return;

    job->id = 0;
    job->pgid = 0;
    job->status = JOB_RUNNING;
    memset(job->command, 0, sizeof(job->command));
    job->processes = NULL;
    memset(&job->terminal_modes, 0, sizeof(job->terminal_modes));
    job->terminal_modes_valid = 0;
    job->next = NULL;
}


void jobmanager_init(JobManager *manager)
{
    if (!manager) return;

    manager->head = NULL;
    manager->nextid = 1;
}

Job *job_add(JobManager *manager, pid_t pgid, const char *command)
{
    if (!manager || pgid <= 0 || !command) {
        log_error("job_add: invalid argument");
        return NULL;
    }

    Job *job = malloc(sizeof(Job));

    if (!job) {
        log_error("failed malloc job");
        return NULL;
    }

    job_init(job);

    job->id = manager->nextid++;
    job->pgid = pgid;

    strncpy(job->command, command, sizeof(job->command) - 1);
    job->command[sizeof(job->command) - 1] = '\0';

    job->next = manager->head;
    manager->head = job;

    return job;
}

Job *job_find(JobManager *manager, pid_t pgid)
{
    if (!manager || pgid <= 0) return NULL;

    Job *job = manager->head;

    while (job) {
        if (job->pgid == pgid) return job;
        job = job->next;
    }

    return NULL;
}

int process_add(Job *job, pid_t pid)
{
    if (!job || pid <= 0) {
        log_error("process_add: invalid argument");
        return -1;
    }

    Process *process = malloc(sizeof(Process));

    if (!process) {
        log_error("failed malloc process");
        return -1;
    }

    process->pid = pid;
    process->status = PROCESS_RUNNING;
    process->wait_status = 0;
    process->wait_status_valid = 0;
    process->next = NULL;

    if (!job->processes) {
        job->processes = process;
        return 0;
    }

    Process *current = job->processes;

    while (current->next) current = current->next;

    current->next = process;

    return 0;
}

void job_list(JobManager *manager)
{
    if (!manager) return;

    Job *job = manager->head;

    while (job) {
        printf("[%d] %s\n", job->id, job->command);

        if (job->status == JOB_RUNNING) printf("now it is running\n");
        else if (job->status == JOB_STOPPED) printf("now it is stopped\n");
        else if (job->status == JOB_DONE) printf("now it is done\n");

        job = job->next;
    }
}

int job_count_active(const JobManager *manager)
{
    if (!manager) return 0;

    int count = 0;
    const Job *job = manager->head;

    while (job) {
        if (job->status == JOB_RUNNING || job->status == JOB_STOPPED) count++;
        job = job->next;
    }

    return count;
}

int job_continue(Job *job)
{
    if (!job || job->pgid <= 0) {
        log_error("job_continue: invalid job");
        return -1;
    }

    if (job->status != JOB_STOPPED) {
        log_error("job_continue: job is not stopped");
        return -1;
    }

    while (kill(-job->pgid, SIGCONT) < 0) {
        if (errno == EINTR) continue;

        log_error("job_continue: failed send SIGCONT");
        return -1;
    }

    Process *process = job->processes;

    while (process) {
        if (process->status == PROCESS_STOPPED) process->status = PROCESS_RUNNING;
        process = process->next;
    }

    job_update_status(job);

    return 0;
}

int job_wait_foreground(Job *job)
{
    if (!job || job->pgid <= 0) return -1;

    while (job->status == JOB_RUNNING) {
        int status = 0;
        pid_t pid = waitpid(-job->pgid, &status, WUNTRACED);

        if (pid < 0) {
            if (errno == EINTR) continue;

            log_error("failed wait foreground job");
            return -1;
        }

        Process *process = process_find(job, pid);

        if (!process) {
            log_error("foreground process not found in job");
            return -1;
        }

        process_apply_wait_status(process, status);
        job_update_status(job);
    }

    return 0;
}

int job_exit_status(const Job *job)
{
    Process *process = job_last_process(job);

    if (!process || !process->wait_status_valid) return -1;

    int status = process->wait_status;

    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);

    return -1;
}

void job_reap(JobManager *manager)
{
    if (!manager) return;

    Job *job = manager->head;

    while (job) {
        Process *process = job->processes;

        while (process) {
            if (process->status == PROCESS_DONE) {
                process = process->next;
                continue;
            }

            int status = 0;
            pid_t ret = waitpid(process->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

            if (ret < 0) {
                if (errno == ECHILD) {
                    process->status = PROCESS_DONE;
                } else if (errno != EINTR) {
                    log_error("failed waitpid process %d", process->pid);
                }
            } else if (ret == process->pid) {
                process_apply_wait_status(process, status);

                if (WIFEXITED(status)) {
                    printf("\n[%d] process %d exit %d\n", job->id, process->pid, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("\n[%d] process %d killed by %d\n", job->id, process->pid, WTERMSIG(status));
                } else if (WIFSTOPPED(status)) {
                    printf("\n[%d] process %d stopped by %d\n", job->id, process->pid, WSTOPSIG(status));
                }
            }

            process = process->next;
        }

        job_update_status(job);
        job = job->next;
    }
}

void job_cleanup_done(JobManager *manager)
{
    if (!manager) return;

    Job *current = manager->head;
    Job *prev = NULL;

    while (current) {
        if (current->status == JOB_DONE) {
            Job *next = current->next;

            if (!prev) manager->head = next;
            else prev->next = next;

            process_destroy(current->processes);
            free(current);
            current = next;
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void job_remove(JobManager *manager, pid_t pgid)
{
    if (!manager || pgid <= 0) return;

    Job *current = manager->head;
    Job *prev = NULL;

    while (current) {
        if (current->pgid == pgid) {
            if (!prev) manager->head = current->next;
            else prev->next = current->next;

            process_destroy(current->processes);
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }
}

void job_destroy(JobManager *manager)
{
    if (!manager) return;

    Job *job = manager->head;

    while (job) {
        Job *next = job->next;
        process_destroy(job->processes);
        free(job);
        job = next;
    }

    manager->head = NULL;
    manager->nextid = 1;
}

void job_shutdown(JobManager *manager)
{
    if (!manager) return;

    if (!manager->head) {
        manager->nextid = 1;
        return;
    }

    Job *job = manager->head;

    while (job) {
        if (job->status == JOB_STOPPED)
            continue_stopped_job(job);

        job = job->next;
    }

    job = manager->head;

    while (job) {
        if (job->status != JOB_DONE && job->pgid > 0) {
            if (kill(-job->pgid, SIGTERM) < 0 && errno != ESRCH) {
                log_error("failed terminate job %d pgid %d during shutdown: %s",
                          job->id, job->pgid, strerror(errno));
            }
        }

        job = job->next;
    }

    for (int i = 0; i < JOB_SHUTDOWN_RETRY_COUNT; i++) {
        if (jobmanager_try_reap_all(manager))
            break;

        shutdown_sleep();
    }

    job = manager->head;

    while (job) {
        if (job->pgid > 0) {
            if (kill(-job->pgid, SIGKILL) < 0 && errno != ESRCH) {
                log_error("failed kill job %d pgid %d during shutdown: %s",
                          job->id, job->pgid, strerror(errno));
            }
        }

        job = job->next;
    }

    job = manager->head;

    while (job) {
        Process *process = job->processes;

        while (process) {
            process_force_reap(process);
            process = process->next;
        }

        job_update_status(job);
        job = job->next;
    }

    job_destroy(manager);
}

