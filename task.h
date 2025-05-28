#ifndef TASK_H
#define TASK_H

typedef struct {
    char *name;
    int tid;
    int priority;
    int burst;
    int deadline;
    int waiting_time;
    int remaining_burst;
    int start_time; // nao usada
    int arrival_time; // nao usada
} Task;


#endif
