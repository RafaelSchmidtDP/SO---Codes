// schedulers_rr.c - Round-Robin clássico Scheduler Implementation
#include "schedulers_rr.h"
#include "list.h"
#include "CPU.h"
#include <pthread.h>

// variáveis do timer (extern declaradas em driver.c)
// timer
extern volatile int slice_expired;
extern pthread_mutex_t timer_mutex;
extern pthread_cond_t  timer_cond;

// cabeçote da fila de aptos
static struct node *ready = NULL;

void add(Task *t) {
    insert(&ready, t);
}

void schedule() {
    while (ready != NULL) {
        Task *t = ready->task;
        delete(&ready, t);

        cpu_run(t);

        // aguarda estouro de fatia
        pthread_mutex_lock(&timer_mutex);
        while (!slice_expired) {
            pthread_cond_wait(&timer_cond, &timer_mutex);
        }
        slice_expired = 0;
        pthread_mutex_unlock(&timer_mutex);

        cpu_preempt();

        t->burst -= 10;
        if (t->burst > 0) {
            add(t);
        } else {
            free(t->name);
            free(t);
        }
    }
}
