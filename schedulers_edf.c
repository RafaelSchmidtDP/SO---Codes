#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "schedulers_edf.h"
#include "timer.h"  // 

#define TIME_QUANTUM 10  // Quantum de execução por task (em ticks)

// Lista de tarefas
struct node *task_list = NULL;

// Função para adicionar uma task (com deadline)
void add(char *name, int priority, int burst, int deadline) {
    Task *newTask = malloc(sizeof(Task));
    newTask->name = strdup(name);
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->remaining_burst = burst;
    newTask->deadline = deadline;
    newTask->start_time = get_global_time();  // Marca o tempo atual

    insert_at_tail(&task_list, newTask);
}

// Função auxiliar para buscar a task com menor deadline
Task *get_task_with_earliest_deadline() {
    if (task_list == NULL) return NULL;

    struct node *temp = task_list;
    Task *earliest = temp->task;

    while (temp != NULL) {
        if (temp->task->deadline < earliest->deadline) {
            earliest = temp->task;
        }
        temp = temp->next;
    }

    return earliest;
}

// Função principal do escalonador EDF
void schedule() {
    timer_set_quantum(10);   // Quantum = 1 tick = 100ms
    timer_start();

    while (task_list != NULL) {
        timer_wait_quantum_expired();

        int clock_time = get_global_time();

        printf("===== Clock Time: %d =====\n", clock_time);

        // Verificar deadlines
        struct node *temp = task_list;
        while (temp != NULL) {
            if (clock_time > temp->task->deadline) {
                printf("⚠️  Task %s perdeu o deadline! (Deadline: %d)\n", 
                       temp->task->name, temp->task->deadline);
            }
            temp = temp->next;
        }

        // Seleciona a task com menor deadline
        Task *t = get_task_with_earliest_deadline();
        if (t == NULL) continue;

        int exec_time = (t->remaining_burst < TIME_QUANTUM) ? 
                        t->remaining_burst : TIME_QUANTUM;

        run(t, exec_time);

        t->remaining_burst -= exec_time;

        if (t->remaining_burst <= 0) {
            printf("✅ Task %s finalizada.\n", t->name);
            delete_task(&task_list, t);
        }
    }

    timer_stop();
}
