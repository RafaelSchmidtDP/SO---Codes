#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "schedulers_rr_p.h"
#include "timer.h"

#define QUANTUM 2   // Quantum em unidades de tempo simuladas

// Filas separadas por prioridade
struct node *filas[MAX_PRIORITY] = { NULL };

// Gera IDs únicos para cada tarefa
int tid_counter = 1;

void add(char *name, int priority, int burst) {
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        printf("Erro: Prioridade inválida (%d). Deve estar entre %d e %d.\n",
               priority, MIN_PRIORITY, MAX_PRIORITY);
        return;
    }

    // Cria a task diretamente
    Task *task = (Task *)malloc(sizeof(Task));
    task->name = strdup(name);  // Copia o nome da task
    task->priority = priority;
    task->burst = burst;
    task->tid++;;  // Se você usa tid, ajusta aqui. Se não usar, pode remover.

    // Insere na fila correspondente à prioridade
    insert_at_tail(&filas[priority - 1], task);       // verificar isso
}


void schedule() {
    int tarefasRestantes = 0;
    for (int i = 0; i < MAX_PRIORITY; i++) {
        struct node *temp = filas[i];
        while (temp != NULL) {
            tarefasRestantes++;
            temp = temp->next;
        }
    }

    while (tarefasRestantes > 0) {
        int encontrouTarefa = 0;

        Task *tarefasFinalizadas[100];
        int finalizadasCount = 0;

        for (int i = 0; i < MAX_PRIORITY; i++) {
            while (filas[i] != NULL) {
                encontrouTarefa = 1;

                Task *tarefa = remove_first_task(&filas[i]);
                int slice = (tarefa->burst > QUANTUM) ? QUANTUM : tarefa->burst;

                timer_set_quantum(slice);
                timer_start();

                run(tarefa, slice);

                timer_wait_quantum_expired();
                timer_stop();

                tarefa->burst -= slice;

                if (tarefa->burst > 0) {
                    insert_at_tail(&filas[i], tarefa);
                } else {
                    tarefasFinalizadas[finalizadasCount++] = tarefa;
                    tarefasRestantes--;
                }
            }
        }

        printf("\n");

        for (int j = 0; j < finalizadasCount; j++) {
            printf("Task [%s] (TID: %d) concluída.\n", tarefasFinalizadas[j]->name, tarefasFinalizadas[j]->tid);
            free_task(tarefasFinalizadas[j]);
        }

        if (!encontrouTarefa) {
            printf("Nenhuma tarefa disponível. Avançando tempo ocioso.\n");
            break;
        }
    }
}