// task.h  (use este em todos os módulos)
#ifndef TASK_H
#define TASK_H

typedef struct {
    char *name;
    int tid;
    int priority;
    int burst;
    int deadline;
    int waiting_time;
    int periodo;
    int arrival;

    int remaining;
    int next_release;
    int release_count;
} Task;


#endif // TASK_H
