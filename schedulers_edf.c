// schedulers_edf.c - Earliest Deadline First Scheduler Implementation
#include "schedulers_edf.h"
#include "list.h"
#include "CPU.h"
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>

// timer
// timer
extern volatile int slice_expired;
extern pthread_mutex_t timer_mutex;
extern pthread_cond_t  timer_cond;


// fila única
static struct node *ready = NULL;

void add(Task *t) {
    insert(&ready, t);
}

void schedule() {
    while (ready != NULL) {
        struct node *it = ready;
        Task *best = it->task;
        int best_dead = best->deadline;
        it = it->next;
        while (it) {
            if (it->task->deadline < best_dead) {
                best = it->task;
                best_dead = best->deadline;
            }
            it = it->next;
        }
        delete(&ready, best);

        cpu_run(best);

        // espera fatia
        pthread_mutex_lock(&timer_mutex);
        while (!slice_expired)
            pthread_cond_wait(&timer_cond, &timer_mutex);
        slice_expired = 0;
        pthread_mutex_unlock(&timer_mutex);

        cpu_preempt();

        best->burst -= 10;
        if (best->burst > 0) {
            add(best);
        } else {
            free(best->name);
            free(best);
        }
    }
}
