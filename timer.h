#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>

void timer_start(void);
void timer_stop(void);
// Espera até o próximo tick (100ms)
void timer_wait_tick(void);
// Espera até o quantum (slice) expirar
void timer_wait_quantum_expired(void);
int get_global_time(void);
void timer_set_quantum(int q);
#endif // TIMER_H
