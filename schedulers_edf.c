#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "timer.h"

// Lista de tarefas
struct node *task_list = NULL;

// Gerador de IDs
int tid_counter = 1;

// Adiciona uma tarefa
void add(char *name, int priority, int burst, int deadline) {
    Task *newTask = (Task *) malloc(sizeof(Task));
    newTask->name = strdup(name);
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->tid = tid_counter++;
    newTask->deadline = deadline;
    newTask->waiting_time = 0;
    newTask->arrival_time = time_elapsed; 
    newTask->start_time = -1;

    printf("[ADD] Task [%s] (TID: %d) added at time %d (deadline in %d)\n",
            newTask->name, newTask->tid, newTask->arrival_time, newTask->deadline);

    insert_at_tail(&task_list, newTask);
}

// Calcula tempo restante até o deadline absoluto
int time_to_deadline(Task *task) {
    int absolute_deadline = task->arrival_time + task->deadline;
    return absolute_deadline - time_elapsed;
}

// Busca a tarefa com deadline mais próximo
Task* find_earliest_deadline() {
    if (task_list == NULL) return NULL;

    struct node *temp = task_list;
    Task *earliest = temp->task;
    int earliest_time = time_to_deadline(earliest);

    while (temp != NULL) {
        int current_time = time_to_deadline(temp->task);
        if (current_time < earliest_time) {
            earliest = temp->task;
            earliest_time = current_time;
        }
        temp = temp->next;
    }
    return earliest;
}

// Imprime o estado atual da fila
void print_task_list() {
    printf("🔍 Tasks in the system:\n");
    struct node *temp = task_list;
    while (temp != NULL) {
        int abs_deadline = temp->task->arrival_time + temp->task->deadline;
        int ttd = time_to_deadline(temp->task);
        printf(" - [%s] (TID:%d) | Burst:%d | Deadline:%d (absolute at %d) | Time to deadline:%d | Start:%d | Arrival:%d\n",
            temp->task->name,
            temp->task->tid,
            temp->task->burst,
            temp->task->deadline,
            abs_deadline,
            ttd,
            temp->task->start_time,
            temp->task->arrival_time
        );
        temp = temp->next;
    }
}

// Função principal EDF
void schedule() {
    start_timer();

    while (task_list != NULL) {
        printf("\n================== Scheduler Status at time %d ==================\n", time_elapsed);
        print_task_list();

        Task *task = find_earliest_deadline();

        if (task == NULL) {
            printf("No task found\n");
            break;
        }

        int remaining_deadline = time_to_deadline(task);
        int absolute_deadline = task->arrival_time + task->deadline;

        // Marca o timestamp de início
        if (task->start_time == -1) {
            task->start_time = time_elapsed;
        }

        if (remaining_deadline < 0) {
            printf("Task [%s] (TID:%d) MISSED its deadline! (Deadline was at time %d, current time %d)\n",
                task->name, task->tid, absolute_deadline, time_elapsed);
        } else {
            printf("Selected task: [%s] (TID:%d) with earliest deadline at time %d (in %d units)\n",
                task->name, task->tid, absolute_deadline, remaining_deadline);
            printf("Running task [%s] for %d units. Start time: %d\n", task->name, task->burst, task->start_time);
        }

        run(task, task->burst);
        time_elapsed += task->burst;
        // Atualiza tempo de espera das outras tarefas
        struct node *temp = task_list;
        while (temp != NULL) {
            if (temp->task != task) {
                temp->task->waiting_time += task->burst;
            }
            temp = temp->next;
        }

        delete_task(&task_list, task);
        free_task(task);

        printf("===============================================================\n");
    }

    printf("\nAll tasks completed at time %d\n", time_elapsed);
    stop_timer();
}
