#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "timer.h"

int time_elapsed = 0;
pthread_t timer_thread_id;
int running = 0;

void *timer_thread(void *arg) {
    while (running) {
        sleep(1); // Simula 1 unidade de tempo (1 segundo)
        time_elapsed++;
        // Aqui poderia disparar flags, sinais, interrupções simuladas
    }
    return NULL;
}

void start_timer() {
    running = 1;
    time_elapsed = 0;
    pthread_create(&timer_thread_id, NULL, timer_thread, NULL);
}

void stop_timer() {
    running = 0;
    pthread_join(timer_thread_id, NULL);
}
