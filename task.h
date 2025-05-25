// task.h  (use este em todos os módulos)
#ifndef TASK_H
#define TASK_H

typedef struct {
    char *name;
    int tid;
    int priority;
    int burst;
    int waiting_time;
    int remaining;
    int deadline;
    int periodo;
    int next_release;
} Task;

#endif // TASK_H
