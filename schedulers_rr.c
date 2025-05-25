// schedule_rr.c
#include <stdlib.h> // Para malloc e free
#include <stdio.h>  // Para printf (para debug ou mensagens)

#include "schedulers_rr.h" // Inclui seu próprio cabeçalho
#include "task.h"        // Para a estrutura Task
#include "list.h"        // Para as operações de lista
#include "CPU.h"         // Para a função run() da CPU

// Definindo o quantum de tempo (máximo 10 unidades)
#define QUANTUM 10

// Fila de aptos para o escalonador Round-Robin.
// É uma variável estática global para este arquivo .c, mantendo-a "viva"
// durante toda a execução do escalonador RR.
static struct node *ready_queue = NULL;

// Variável para gerar IDs únicos para as tarefas (opcional, se não vier do arquivo de entrada)
static int tid_counter = 0;

// add: Adiciona uma nova tarefa à fila de aptos.
// Para RR clássico, a prioridade não é usada na lógica de escalonamento,
// mas é armazenada na tarefa para consistência com a Task struct.
void add(char *name, int priority, int burst) {
    // Aloca memória para a nova tarefa
    Task *newTask = (Task*) malloc(sizeof(Task));
    if (newTask == NULL) {
        perror("Erro ao alocar memória para a Task");
        exit(EXIT_FAILURE);
    }

    // Preenche os campos da tarefa
    newTask->name = strdup(name); // Copia o nome da string
    if (newTask->name == NULL) {
        perror("Erro ao alocar memória para o nome da Task");
        free(newTask);
        exit(EXIT_FAILURE);
    }
    newTask->tid = ++tid_counter; // Atribui um TID único
    newTask->priority = priority;
    newTask->burst = burst;
    newTask->deadline = 0; // Não usado no RR clássico
    newTask->waiting_time = 0; // Usado para aging, mas inicializado aqui para consistência

    // Adiciona a tarefa ao final da fila de aptos (FIFO para RR)
    insert_at_tail(&ready_queue, newTask);
    printf("Task %s adicionada à fila RR. Burst: %d\n", newTask->name, newTask->burst);
}

// schedule: Invoca a lógica de escalonamento Round-Robin.
void schedule() {
    printf("\nIniciando escalonamento Round-Robin...\n");
    
    // Loop principal do escalonador
    while (ready_queue != NULL) {
        Task *current_task = remove_first_task(&ready_queue); // Pega a primeira tarefa da fila

        if (current_task == NULL) { // Deveria ser inalcançável se o loop while(ready_queue != NULL) for verdadeiro
            break;
        }

        int slice_to_run;
        if (current_task->burst > QUANTUM) {
            slice_to_run = QUANTUM; // Executa pelo quantum total
        } else {
            slice_to_run = current_task->burst; // Executa pelo burst restante
        }

        // Simula a execução da tarefa pela CPU
        run(current_task, slice_to_run);

        current_task->burst -= slice_to_run; // Decrementa o burst da tarefa

        if (current_task->burst > 0) {
            // Se a tarefa não terminou, adiciona-a de volta ao final da fila
            printf("Task %s (Burst restante: %d) volta para o final da fila.\n", 
                   current_task->name, current_task->burst);
            insert_at_tail(&ready_queue, current_task);
        } else {
            // Se a tarefa terminou, libera a memória
            printf("Task %s (TID: %d) concluída.\n", current_task->name, current_task->tid);
            free(current_task->name); // Libera o nome
            free(current_task);        // Libera a Task
        }
        // Aqui você integraria a lógica do timer/thread para pausar e verificar a flag
        // de estouro de tempo se estivesse fazendo uma simulação em tempo real.
        // Por enquanto, é uma simulação lógica instantânea.
    }
    printf("\nEscalonamento Round-Robin concluído. Todas as tarefas foram executadas.\n");
}