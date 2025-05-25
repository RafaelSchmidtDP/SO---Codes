// schedule_rr_p.c
#include <stdlib.h> // Para malloc e free
#include <stdio.h>  // Para printf (para debug ou mensagens)
#include <string.h> // Para strdup

#include "schedulers_rr_p.h" // Inclui seu próprio cabeçalho
#include "task.h"          // Para a estrutura Task
#include "list.h"          // Para as operações de lista
#include "CPU.h"           // Para a função run() da CPU

// Definindo o quantum de tempo (máximo 10 unidades)
#define QUANTUM 10

// Array de filas de aptos para o escalonador Round-Robin com Prioridade.
// O índice do array pode corresponder à prioridade (ajustado se MIN_PRIORITY não for 1).
// Ex: priority_queues[0] para prioridade 1 (se MIN_PRIORITY=1).
static struct node *priority_queues[MAX_PRIORITY + 1]; // +1 para usar o índice 1 a MAX_PRIORITY

// Variável para gerar IDs únicos para as tarefas
static int tid_counter = 0;

// Função de inicialização das filas de prioridade (chamada uma vez antes de add/schedule)
static void init_priority_queues() {
    for (int i = MIN_PRIORITY; i <= MAX_PRIORITY; i++) {
        priority_queues[i] = NULL;
    }
}

// add: Adiciona uma nova tarefa à fila de aptos da sua prioridade correta.
void add(char *name, int priority, int burst) {
    // Inicializa as filas na primeira chamada (ou você pode chamar init_priority_queues em main)
    static int initialized = 0;
    if (!initialized) {
        init_priority_queues();
        initialized = 1;
    }

    // Valida a prioridade
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        fprintf(stderr, "Erro: Prioridade %d fora da faixa [%d, %d] para a task %s.\n",
                priority, MIN_PRIORITY, MAX_PRIORITY, name);
        return;
    }

    // Aloca memória para a nova tarefa
    Task *newTask = (Task*) malloc(sizeof(Task));
    if (newTask == NULL) {
        perror("Erro ao alocar memória para a Task");
        exit(EXIT_FAILURE);
    }

    // Preenche os campos da tarefa
    newTask->name = strdup(name);
    if (newTask->name == NULL) {
        perror("Erro ao alocar memória para o nome da Task");
        free(newTask);
        exit(EXIT_FAILURE);
    }
    newTask->tid = ++tid_counter;
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->deadline = 0; // Não usado no RR_p
    newTask->waiting_time = 0; // Usado para aging, mas inicializado aqui para consistência

    // Adiciona a tarefa à fila de sua prioridade.
    // Usamos insert_at_tail para manter o comportamento FIFO para tarefas de mesma prioridade.
    insert_at_tail(&priority_queues[priority], newTask);
    printf("Task %s (Prioridade: %d, Burst: %d) adicionada à fila de prioridade.\n",
           newTask->name, newTask->priority, newTask->burst);
}

// schedule: Invoca a lógica de escalonamento Round-Robin com Prioridade.
void schedule() {
    printf("\nIniciando escalonamento Round-Robin com Prioridade...\n");

    // Loop principal do escalonador. Continua enquanto houver alguma tarefa em qualquer fila.
    while (1) {
        Task *current_task = NULL;
        int current_priority = -1;

        // Itera pelas filas de prioridade, da maior (1) para a menor (10)
        for (int i = MIN_PRIORITY; i <= MAX_PRIORITY; i++) {
            if (priority_queues[i] != NULL) {
                // Encontrou uma fila não vazia na maior prioridade atual
                current_task = remove_first_task(&priority_queues[i]);
                current_priority = i;
                break; // Sai do loop, pois encontrou a tarefa de maior prioridade
            }
        }

        if (current_task == NULL) {
            // Se nenhuma tarefa foi encontrada em nenhuma fila, todas as filas estão vazias.
            break; // Sai do loop principal
        }

        // Lógica de execução da tarefa
        int slice_to_run;
        if (current_task->burst > QUANTUM) {
            slice_to_run = QUANTUM;
        } else {
            slice_to_run = current_task->burst;
        }

        // Simula a execução da tarefa
        run(current_task, slice_to_run);

        current_task->burst -= slice_to_run; // Decrementa o burst

        if (current_task->burst > 0) {
            // Se a tarefa não terminou, adiciona-a de volta ao final de SUA fila de prioridade
            printf("Task %s (Prioridade: %d, Burst restante: %d) volta para o final da sua fila.\n",
                   current_task->name, current_task->priority, current_task->burst);
            insert_at_tail(&priority_queues[current_priority], current_task);
        } else {
            // Se a tarefa terminou, libera a memória
            printf("Task %s (TID: %d, Prioridade: %d) concluída.\n",
                   current_task->name, current_task->tid, current_task->priority);
            free(current_task->name);
            free(current_task);
        }
        // Aqui você integraria a lógica do timer/thread.
    }
    printf("\nEscalonamento Round-Robin com Prioridade concluído. Todas as tarefas foram executadas.\n");
}