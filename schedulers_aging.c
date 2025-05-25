// schedulers_aging.c - Prioridade com Aging Scheduler Implementation
#include "schedulers_aging.h"
#include "CPU.h"
#include <pthread.h>
#include <stdlib.h>

// timer
// timer
extern volatile int slice_expired;
extern pthread_mutex_t timer_mutex;
extern pthread_cond_t  timer_cond;


// várias filas por prioridade
static struct node *queues[MAX_PRIORITY+1] = { NULL };

static int any_queue() {
    for (int p = MIN_PRIORITY; p <= MAX_PRIORITY; p++)
        if (queues[p]) return 1;
    return 0;
}

void add(Task *t) {
    if (t->priority < MIN_PRIORITY) t->priority = MIN_PRIORITY;
    if (t->priority > MAX_PRIORITY) t->priority = MAX_PRIORITY;
    t->waiting_time = 0;
    insert(&queues[t->priority], t);
}

void schedule() {
    while (any_queue()) {
        for (int p = MIN_PRIORITY; p <= MAX_PRIORITY; p++) {
            struct node *it = queues[p];
            while (it) {
                it->task->waiting_time += 10;
                if (it->task->waiting_time >= AGING_THRESHOLD && it->task->priority > MIN_PRIORITY) {
                    Task *ta = it->task;
                    it = it->next;
                    delete(&queues[p], ta);
                    ta->priority--;
                    ta->waiting_time = 0;
                    insert(&queues[ta->priority], ta);
                } else {
                    it = it->next;
                }
            }
        }

        Task *t = NULL;
        for (int p = MIN_PRIORITY; p <= MAX_PRIORITY; p++) {
            if (queues[p]) {
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
