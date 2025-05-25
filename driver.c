#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "schedulers.h"
#include "CPU.h"

#define TIME_SLICE_UNITS 10

volatile int tick_count = 0;
volatile int slice_expired = 0;
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  timer_cond  = PTHREAD_COND_INITIALIZER;

void *timer_thread(void *arg) {
    const useconds_t unit_duration_us = 1000;
    while (1) {
        usleep(unit_duration_us);
        pthread_mutex_lock(&timer_mutex);
        tick_count++;
        if (tick_count >= TIME_SLICE_UNITS) {
            slice_expired = 1;
            tick_count = 0;
            pthread_cond_signal(&timer_cond);
        }
        pthread_mutex_unlock(&timer_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_de_tasks>\n", argv[0]);
        exit(1);
    }
    pthread_t tid;
    if (pthread_create(&tid, NULL, timer_thread, NULL) != 0) {
        perror("Erro ao criar timer_thread");
        exit(1);
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror("fopen"); exit(1); }

    char name[64];
    int tid_i, priority, burst, deadline;
    while (fscanf(f, "%63s %d %d %d %d", name, &tid_i, &priority, &burst, &deadline) == 5) {
        Task *t = malloc(sizeof(Task));
        t->name = strdup(name);
        t->tid = tid_i;
        t->priority = priority;
        t->burst = burst;
        t->deadline = deadline;
        t->waiting_time = 0;
        add(t);
    }
    fclose(f);

    schedule();
    return 0;
}
