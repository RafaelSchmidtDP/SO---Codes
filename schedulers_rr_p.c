// schedulers_rr_p.c - Round-Robin com Prioridade Scheduler Implementation
#include "schedulers_rr_p.h"
#include "CPU.h"
#include <pthread.h>
#include <stdlib.h>

// timer
// timer
extern volatile int slice_expired;
extern pthread_mutex_t timer_mutex;
extern pthread_cond_t  timer_cond;


// um array de filas, index = prioridade
static struct node *queues[MAX_PRIORITY+1] = { NULL };

static int any_queue_nonempty() {
    for (int p = MIN_PRIORITY; p <= MAX_PRIORITY; p++)
        if (queues[p] != NULL) return 1;
    return 0;
}

void add(Task *t) {
    int p = t->priority;
    if (p < MIN_PRIORITY) p = MIN_PRIORITY;
    if (p > MAX_PRIORITY) p = MAX_PRIORITY;
    insert(&queues[p], t);
}

void schedule() {
    while (any_queue_nonempty()) {
        Task *t = NULL;
        // escolhe maior prioridade disponível
        for (int p = MIN_PRIORITY; p <= MAX_PRIORITY; p++) {
            if (queues[p] != NULL) {
                struct node *n = queues[p];
                queues[p] = n->next;
                t = n->task;
                free(n);
                break;
            }
        }

        cpu_run(t);

        // espera fatia
        pthread_mutex_lock(&timer_mutex);
        while (!slice_expired)
            pthread_cond_wait(&timer_cond, &timer_mutex);
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
