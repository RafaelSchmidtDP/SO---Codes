#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "schedulers_rr_p.h"

#define QUANTUM 10   // Defina aqui o quantum desejado
#define NUM_PRIORIDADES (MAX_PRIORITY - MIN_PRIORITY + 1)

// Filas separadas por prioridade
struct node *filas[NUM_PRIORIDADES] = { NULL };

// Gera IDs únicos para cada tarefa
int tid_counter = 1;

// Adiciona uma tarefa na fila correspondente à sua prioridade
void add(char *name, int priority, int burst) {
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        printf("Erro: Prioridade inválida (%d). Deve estar entre %d e %d.\n",
               priority, MIN_PRIORITY, MAX_PRIORITY);
        return;
    }

    Task *task = create_task(name, tid_counter++, priority, burst, 0, 0);

    int idx = MAX_PRIORITY - priority; // Inverte prioridade (1 = mais alta)
    insert_at_tail(&filas[idx], task);
}

// Função principal de escalonamento RR com prioridades
void schedule() {
    printf("\n--- Escalonamento Round Robin com Prioridade ---\n");

    int tarefasRestantes = 0;

    // Conta total de tarefas inicialmente
    for (int i = 0; i < NUM_PRIORIDADES; i++) {
        struct node *temp = filas[i];
        while (temp != NULL) {
            tarefasRestantes++;
            temp = temp->next;
        }
    }

    while (tarefasRestantes > 0) {
        int encontrouTarefa = 0;

        // Percorre das prioridades mais altas (índice menor) para as mais baixas
        for (int i = 0; i < NUM_PRIORIDADES; i++) {
            if (filas[i] != NULL) {
                encontrouTarefa = 1;

                Task *tarefa = remove_first_task(&filas[i]);
                int slice = (tarefa->burst > QUANTUM) ? QUANTUM : tarefa->burst;

                run(tarefa, slice);

                tarefa->burst -= slice;

                if (tarefa->burst > 0) {
                    // Tarefa não terminou, volta para o fim da fila da mesma prioridade
                    insert_at_tail(&filas[i], tarefa);
                } else {
                    // Tarefa concluída
                    printf("Task [%s] (TID: %d) concluída.\n", tarefa->name, tarefa->tid);
                    free_task(tarefa);
                    tarefasRestantes--;
                }

                // Executa apenas uma tarefa por vez na prioridade atual
                break;
            }
        }

        if (!encontrouTarefa) {
            printf("Nenhuma tarefa disponível. Avançando tempo ocioso.\n");
            break;
        }
    }

    printf("\n--- Fim do Escalonamento ---\n");
}
