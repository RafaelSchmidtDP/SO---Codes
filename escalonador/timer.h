#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>

extern int time_elapsed; 

void stop_timer();
void start_timer();

#endif // TIMER_H
