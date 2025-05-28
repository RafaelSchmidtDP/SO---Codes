#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "CPU.h"
#include "task.h"
#include "timer.h"
#include "schedulers_aging.h"

// Configurações
#define QUANTUM 5
#define LIMIAR_AGING 10
#define MAX_PRIORITY 10
#define MIN_PRIORITY 1


// Inicializa as filas de prioridade
struct node *filas[MAX_PRIORITY] = { NULL };


void add(char *name, int priority, int burst) {
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        printf("Erro: Prioridade inválida (%d). Deve estar entre %d e %d.\n",
               priority, MIN_PRIORITY, MAX_PRIORITY);
        return;
    }

    Task *task = malloc(sizeof(Task));
    task->name = strdup(name);
    task->priority = priority;
    task->burst = burst;
    task->waiting_time = time_elapsed; // Marca quando a task entra no sistema

    insert_at_tail(&filas[priority - 1], task);
}

Task *proximo_processo() {
    for (int i = 0; i < MAX_PRIORITY; i++) {
        if (filas[i] != NULL) {
            return filas[i]->task;
        }
    }
    return NULL;
}

void aging(Task *executando) {
    for (int i = 1; i < MAX_PRIORITY; i++) {
        struct node *temp = filas[i];
        while (temp != NULL) {
            Task *t = temp->task;
            struct node *next = temp->next; 

            if (t != executando) {
                int tempo_esperando = time_elapsed - t->waiting_time;

                if (tempo_esperando >= LIMIAR_AGING && t->priority > MIN_PRIORITY) {
                    // Remove da fila atual
                    delete_task(&filas[i], t);

                    // Promove a prioridade
                    t->priority--;
                    t->waiting_time = time_elapsed;

                    // Insere na fila de prioridade superior
                    insert_at_tail(&filas[i - 1], t);

                    printf("Aging aplicado: [%s] sobe para prioridade %d\n",
                           t->name, t->priority);
                }
            }

            temp = next;
        }
    }
}


void imprimir_filas_e_execucao(Task *executando) {
    printf("\nEstado das filas:\n");
    for (int i = 0; i < MAX_PRIORITY; i++) {
        printf("Fila prioridade %d: ", i + 1);
        if (filas[i] == NULL) {
            printf("(vazia)\n");
            continue;
        }
        struct node *temp = filas[i];
        while (temp != NULL) {
            Task *t = temp->task;
            printf("[%s (Burst %d)] ", t->name, t->burst);
            temp = temp->next;
        }
        printf("\n");
    }

    if (executando != NULL) {
        printf("Executando: [%s] (Prioridade %d, Burst restante %d)\n",
               executando->name, executando->priority, executando->burst);
    } else {
        printf("Nenhuma tarefa está sendo executada no momento.\n");
    }
    printf("\n");
}

void schedule() {
    printf("\n>>> Início do escalonamento (Aging com múltiplas filas)\n");

    start_timer();

    while (1) {
        Task *task = proximo_processo();
        if (task == NULL) break;

        imprimir_filas_e_execucao(task);

        int exec_ticks = (task->burst < QUANTUM) ? task->burst : QUANTUM;
        int start_exec_time = time_elapsed;

        // Executa até quantum ou fim do burst
        while (time_elapsed - start_exec_time < exec_ticks) {
            sleep(1); // espera o tempo avançar junto com o timer
        }

        task->burst -= exec_ticks;

        printf("Tempo %d: [%s] executada (Prioridade %d, Restante %d)\n",
               time_elapsed, task->name, task->priority, task->burst);

        // Aplica aging após execução
        aging(task);

        if (task->burst <= 0) {
            printf("Tarefa [%s] finalizada!\n", task->name);
            int fila_idx = task->priority - 1;
            delete_task(&filas[fila_idx], task);
            free_task(task);
        }
    }

    stop_timer();

    printf(">>> Fim do escalonamento (Aging)\n");
}
