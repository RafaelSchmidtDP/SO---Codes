// schedule_rr.h
#ifndef SCHEDULE_RR_H
#define SCHEDULE_RR_H

#include "task.h" 


#define MIN_PRIORITY 1
#define MAX_PRIORITY 10

void add(char *name, int priority, int burst);


void schedule();

#endif // SCHEDULE_RR_H