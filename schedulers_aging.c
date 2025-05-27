#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "CPU.h"
#include "timer.h"
#include "task.h"
#include "schedulers_aging.h"

#define QUANTUM 3
#define LIMIAR_AGING 1
#define MAX_PRIORITY 10
#define MIN_PRIORITY 1

// Inicializa todas as filas como NULL já direto aqui
struct node *filas[MAX_PRIORITY] = { NULL };  

// Adiciona tarefa na fila correta pela prioridade
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
    task->waiting_time = 0;

    // Insere no final da fila correspondente
    insert_at_tail(&filas[priority - 1], task);
}

// Retorna o próximo processo da maior prioridade com tarefas pendentes
Task *proximo_processo() {
    for (int i = 0; i < MAX_PRIORITY; i++) {
        if (filas[i] != NULL) {
            return filas[i]->task;  // primeira tarefa da fila i
        }
    }
    return NULL;  // todas filas vazias
}

// Aplica envelhecimento promovendo tarefas para fila superior
void aging(Task *executando) {
    for (int i = 1; i < MAX_PRIORITY; i++) { // começa de 1 (prioridade 2)
        struct node *temp = filas[i];
        while (temp != NULL) {
            Task *t = temp->task;
            if (t != executando) {
                t->waiting_time++;

                if (t->waiting_time >= LIMIAR_AGING && t->priority > MIN_PRIORITY) {
                    // Remove da fila atual
                    delete_task(&filas[i], t);

                    // Promove prioridade (fila anterior)
                    t->priority--;
                    t->waiting_time = 0;

                    // Insere na fila de prioridade superior
                    insert_at_tail(&filas[i - 1], t);

                    printf("Aging aplicado: [%s] sobe para prioridade %d\n",
                           t->name, t->priority);
                }
            }
            temp = temp->next;
        }
    }
}

// Imprime as filas e a tarefa que está sendo executada
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

// Função principal de escalonamento
void schedule() {
    printf("\n>>> Início do escalonamento (Aging com múltiplas filas)\n");

    timer_start();

    while (1) {
        Task *task = proximo_processo();
        if (task == NULL) break;  // todas filas vazias

        imprimir_filas_e_execucao(task);

        int exec_ticks = (task->burst < QUANTUM) ? task->burst : QUANTUM;

        timer_set_quantum(exec_ticks);
        timer_wait_quantum_expired();

        int tempo = get_global_time();

        task->burst -= exec_ticks;
        task->waiting_time = 0; // reset espera pq está executando

        printf("Tempo %d: [%s] executada (Prioridade %d, Restante %d)\n",
               tempo, task->name, task->priority, task->burst);

        aging(task);

        if (task->burst <= 0) {
            printf("Tarefa [%s] finalizada!\n", task->name);
            // Remove uma tarefa da fila correta, usada após finalizar burst
            int fila_idx = task->priority - 1;
            delete_task(&filas[fila_idx], task);
            free_task(task);
        }
    }

    timer_stop();

    printf(">>> Fim do escalonamento (Aging)\n");
}
