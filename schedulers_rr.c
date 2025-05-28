#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "timer.h"
#include "schedulers_rr.h"  

#define QUANTUM 3   // quantum em ticks (cada tick = 100ms)
struct node *fila = NULL;

// Gera IDs únicos para cada tarefa
int tid_counter = 1;

void add(char *name, int priority, int burst) {
    Task *task = (Task *)malloc(sizeof(Task));

    task->name = strdup(name);  // Copia o nome da task
    task->priority = priority;
    task->burst = burst;
    task->tid = tid_counter++;


    insert_at_tail(&fila, task);
}

void schedule() {
    timer_set_quantum(QUANTUM);
    timer_start();

    while (fila != NULL) {
        Task *tarefa = remove_first_task(&fila);
// quantum 4 - tempo 1 em 1 - 4+
        int slice = (tarefa->burst > QUANTUM) ? QUANTUM : tarefa->burst;
        printf("Executando tarefa [%s] (TID: %d) por até %d ticks...\n", tarefa->name, tarefa->tid, slice);

        for (int i = 0; i < slice; i++) {
            run(tarefa, 1);  // Executa 1 tick de CPU
            tarefa->burst--;

            timer_wait_quantum_expired();  // Espera próximo tick

            if (tarefa->burst <= 0) {
                break;
            }
        }

        if (tarefa->burst > 0) {
            insert_at_tail(&fila, tarefa);
        } else {
            printf("Tarefa [%s] (TID: %d) concluída.\n", tarefa->name, tarefa->tid);
            free_task(tarefa);
        }
    }

    timer_stop();
}