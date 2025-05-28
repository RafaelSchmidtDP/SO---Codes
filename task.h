#ifndef TASK_H
#define TASK_H

typedef struct {
    char *name;
    int tid;
    int priority;
    int burst;
    int deadline;
    int waiting_time;
    int start_time; 
    int arrival_time; 
} Task;


#endif
