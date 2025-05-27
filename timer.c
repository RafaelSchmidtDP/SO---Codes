#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// Thread e sincronização
static pthread_t timer_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_cond_t cond_tick = PTHREAD_COND_INITIALIZER;

static int quantum = 0;           // quantum em ticks (ex: 3 ticks)
static int quantum_counter = 0;   // contador de ticks do quantum atual
static int quantum_expired = 0;   // flag indicando quantum expirado
static int global_time = 0;       // tempo global em ticks desde o start
static int running = 0;           // controle da thread timer

// Função da thread timer (simula hardware timer)
static void *timer_thread_func(void *arg) {
    while (running) {
        usleep(100000);  // 100ms = 1 tick

        pthread_mutex_lock(&mutex);

        global_time++;        // incrementa tempo global
        quantum_counter++;    // incrementa ticks do quantum atual

        if (quantum_counter >= quantum && quantum > 0) {
            quantum_expired = 1;      // quantum acabou
            quantum_counter = 0;
            pthread_cond_signal(&cond_tick);
        } else {
            // Pode avisar ticks intermediários (opcional)
            pthread_cond_signal(&cond_tick);
        }

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void timer_set_quantum(int q) {
    pthread_mutex_lock(&mutex);
    quantum = q;
    quantum_counter = 0;      // reseta contador do quantum para novo valor
    quantum_expired = 0;
    pthread_mutex_unlock(&mutex);
}

void timer_wait_quantum_expired(void) {
    pthread_mutex_lock(&mutex);
    while (!quantum_expired) {
        pthread_cond_wait(&cond_tick, &mutex);
    }
    quantum_expired = 0;
    pthread_mutex_unlock(&mutex);
}

void timer_start(void) {
    pthread_mutex_lock(&mutex);
    global_time = 0;
    running = 1;
    quantum_counter = 0;
    quantum_expired = 0;
    pthread_mutex_unlock(&mutex);

    pthread_create(&timer_thread, NULL, timer_thread_func, NULL);
}

void timer_stop(void) {
    pthread_mutex_lock(&mutex);
    running = 0;
    pthread_cond_signal(&cond_tick);
    pthread_mutex_unlock(&mutex);

    pthread_join(timer_thread, NULL);
}

int get_global_time(void) {
    int t;
    pthread_mutex_lock(&mutex);
    t = global_time;
    pthread_mutex_unlock(&mutex);
    return t;
}
