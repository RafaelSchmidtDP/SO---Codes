#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "task.h"
#include "list.h"
#include "timer.h"
#include "schedulers_edf.h"

// --------------------------
// Lista de tarefas periódicas
// --------------------------
PeriodicTask tasks[] = {
    {.task = NULL, .period = 50},   // Task A
    {.task = NULL, .period = 75},   // Task B
};

int num_tasks = sizeof(tasks) / sizeof(tasks[0]);

// Lista de tarefas prontas
struct node *ready_list = NULL;

// --------------------------
// Função para inicializar as tarefas
// --------------------------
void initialize_tasks() {
    tasks[0].task = create_task("Task A", 1, 25, 50);
    tasks[1].task = create_task("Task B", 1, 30, 75);

    for (int i = 0; i < num_tasks; i++) {
        tasks[i].abs_arrival = 0;
        tasks[i].abs_deadline = tasks[i].period;
        tasks[i].remaining_burst = tasks[i].task->burst;
    }
}

// --------------------------
// Checa se há chegada de tarefa
// --------------------------
void check_task_arrivals(int current_time) {
    for (int i = 0; i < num_tasks; i++) {
        if (current_time == tasks[i].abs_arrival) {
            Task *new_task = create_task(
                tasks[i].task->name,
                tasks[i].task->priority,
                tasks[i].task->burst,
                tasks[i].abs_deadline
            );

            insert_sorted_by_deadline(&ready_list, new_task);
            printf("[TIME %d] Chegada da %s (Deadline: %d)\n", current_time, new_task->name, new_task->deadline);

            // Prepara próximo período
            tasks[i].abs_arrival += tasks[i].period;
            tasks[i].abs_deadline = tasks[i].abs_arrival + tasks[i].period;
        }
    }
}

// --------------------------
// Scheduler EDF
// --------------------------
void *edf_scheduler(void *arg) {
    timer_start();

    while (get_global_time() < 100) {  // Simula até o tempo 100
        int current_time = get_global_time();

        check_task_arrivals(current_time);

        if (ready_list != NULL) {
            Task *current_task = remove_first_task(&ready_list);
            printf("[TIME %d] Executando %s (Deadline: %d)\n", current_time, current_task->name, current_task->deadline);

            // Simula execução do burst inteiro da tarefa
            for (int i = 0; i < current_task->burst; i++) {
                timer_wait_quantum_expired();  // Aguarda 1 unidade de tempo
                current_time = get_global_time();
                check_task_arrivals(current_time);  // Checa se outras tarefas chegaram durante a execução
            }

            printf("[TIME %d] %s finalizada.\n", get_global_time(), current_task->name);
            free_task(current_task);
        } else {
            printf("[TIME %d] CPU Ociosa.\n", current_time);
            timer_wait_quantum_expired();
        }
    }

    timer_stop();
    return NULL;
}
