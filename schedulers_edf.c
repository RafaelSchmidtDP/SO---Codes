#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "list.h"
#include "task.h"
#include "timer.h"

// Ready queue ordenada por deadline
struct node *ready_queue = NULL;

// Task atualmente executando
Task *current_task = NULL;

// Mutex para proteger ready queue e current task
pthread_mutex_t scheduler_mutex = PTHREAD_MUTEX_INITIALIZER;

// Simula a execução da task (gasta 1 tick do burst)
void run_task_for_one_tick(Task *task) {
    if (task->burst > 0) {
        task->burst--;
        printf("Executando %s (tid=%d), burst restante=%d, deadline=%d\n", 
            task->name, task->tid, task->burst, task->deadline);
    }
}

// Insere nova task na ready queue e ordena por deadline
void add_task_to_ready_queue(Task *task) {
    pthread_mutex_lock(&scheduler_mutex);
    insert_sorted_by_deadline(&ready_queue, task);
    pthread_mutex_unlock(&scheduler_mutex);
}

// Remove task da ready queue
void remove_task_from_ready_queue(Task *task) {
    pthread_mutex_lock(&scheduler_mutex);
    delete_task(&ready_queue, task);
    pthread_mutex_unlock(&scheduler_mutex);
}

// Escolhe a task com o menor deadline
Task* pick_task_with_earliest_deadline() {
    pthread_mutex_lock(&scheduler_mutex);
    Task *task = get_first_task(ready_queue);
    pthread_mutex_unlock(&scheduler_mutex);
    return task;
}

// Atualiza o deadline absoluto da task ao executar
void update_task_deadline(Task *task, int current_time, int period) {
    task->deadline = current_time + period;
    task->waiting_time = current_time;
}

// Thread que simula o escalonador EDF
void *scheduler_thread_func(void *arg) {
    while (1) {
        // Espera quantum expirado do timer
        timer_wait_quantum_expired();

        pthread_mutex_lock(&scheduler_mutex);

        int now = get_global_time();

        // Se não tem task atual, pega a primeira da fila
        if (current_task == NULL) {
            current_task = pick_task_with_earliest_deadline();
            if (current_task) {
                printf("Escalonador: escolhendo tarefa %s (deadline=%d)\n", 
                       current_task->name, current_task->deadline);
                remove_task_from_ready_queue(current_task);
            }
        }

        if (current_task) {
            // Executa 1 tick da task atual
            run_task_for_one_tick(current_task);

            // Se task terminou burst, remove da fila e libera
            if (current_task->burst <= 0) {
                printf("Tarefa %s terminou execução.\n", current_task->name);
                free_task(current_task);
                current_task = NULL;
            } else {
                // Verifica se existe task com deadline menor para preempção
                Task *earlier = pick_task_with_earliest_deadline();
                if (earlier && earlier->deadline < current_task->deadline) {
                    printf("Preempção: %s tem deadline menor que %s. Troca de tarefa.\n", 
                           earlier->name, current_task->name);
                    insert_sorted_by_deadline(&ready_queue, current_task);
                    current_task = earlier;
                    remove_task_from_ready_queue(current_task);
                }
            }
        } else {
            printf("Nenhuma tarefa para executar.\n");
        }

        pthread_mutex_unlock(&scheduler_mutex);
    }
    return NULL;
}

int main() {
    // Inicializa timer
    timer_set_quantum(1);  // 1 tick quantum
    timer_start();

    // Cria tarefas de exemplo
    Task *t1 = create_task("P1", 0, 5, 10);
    t1->deadline = 10;
    add_task_to_ready_queue(t1);

    Task *t2 = create_task("P2", 0, 8, 15);
    t2->deadline = 15;
    add_task_to_ready_queue(t2);

    Task *t3 = create_task("P3", 0, 3, 5);
    t3->deadline = 5;
    add_task_to_ready_queue(t3);

    // Cria thread do escalonador
    pthread_t scheduler_thread;
    pthread_create(&scheduler_thread, NULL, scheduler_thread_func, NULL);

    // Espera a thread do escalonador (nunca termina)
    pthread_join(scheduler_thread, NULL);

    timer_stop();

    return 0;
}
