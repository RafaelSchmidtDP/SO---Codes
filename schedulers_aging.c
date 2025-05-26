#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "schedulers_aging.h"
#include "timer.h"  // Incluído para usar o timer e controle de quantum

#define LIMIAR_AGING 2   // Tempo mínimo esperando para subir prioridade
#define MAX_PRIORITY 10
#define MIN_PRIORITY 1
#define QUANTUM 3          // quantum de 3 ticks

struct node *task_list = NULL;  // lista de tarefas

// Adiciona uma nova tarefa à lista
void add(char *name, int priority, int burst) {
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        printf("Erro: Prioridade inválida (%d). Deve estar entre %d e %d.\n",
               priority, MIN_PRIORITY, MAX_PRIORITY);
        return;
    }

    Task *task = create_task(name, priority, burst, 0, 0);
    insert_at_tail(&task_list, task);
}

// Pega a tarefa com a menor prioridade numérica (maior prioridade)
Task *proximo_processo() {
    struct node *temp = task_list;
    Task *menor = NULL;

    while (temp != NULL) {
        if (menor == NULL || temp->task->priority < menor->priority) {
            menor = temp->task;
        }
        temp = temp->next;
    }

    return menor;
}

// Aplica envelhecimento (aging) às tarefas que estão esperando
void aging(Task *executando) {
    struct node *temp = task_list;

    while (temp != NULL) {
        if (temp->task != executando) {
            temp->task->waiting_time+= 1;  // incrementa tempo de espera

            // Se ultrapassar limiar, sobe prioridade (diminui o número)
            if (temp->task->waiting_time >= LIMIAR_AGING && temp->task->priority > MIN_PRIORITY) {
                temp->task->priority--;
                temp->task->waiting_time = 0;
                printf("Aging aplicado: [%s] sobe para prioridade %d\n",
                       temp->task->name, temp->task->priority);
            }
        }
        temp = temp->next;
    }
}

// Função principal de escalonamento
#define QUANTUM 3  // quantum fixo de 3 ticks (300ms)

void schedule() {
    printf("\n>>> Início do escalonamento (Aging)\n");

    timer_start();

    while (task_list != NULL) {
        Task *task = proximo_processo();

        if (task == NULL)
            break;

        // Ajusta quantum para o menor entre QUANTUM e burst restante
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
            delete_task(&task_list, task);
            free_task(task);
        }
    }

    timer_stop();

    printf(">>> Fim do escalonamento (Aging)\n");
}