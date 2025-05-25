#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "schedulers_edf.h"

// Lista global de tasks
struct node *task_list = NULL;

// Variável de tempo global
int tempo_atual = 0;

// Controle do timer
int rodando = 1;

// Thread do timer (simula clock de hardware)
void *timer_function(void *arg) {
    while (rodando) {
        sleep(1);  // Simula 1 unidade de tempo por segundo
        tempo_atual++;
        printf("[TIMER] Tempo atual: %d\n", tempo_atual);
    }
    return NULL;
}

// Função para adicionar uma task
void add(char *name, int priority, int burst, int deadline) {
    static int tid_counter = 1;
    Task *task = create_task(name, tid_counter++, priority, burst, deadline, 0);
    insert(&task_list, task);
}

// Função para encontrar a task com menor deadline
Task *select_task_with_earliest_deadline() {
    struct node *temp = task_list;
    Task *selected = NULL;

    while (temp != NULL) {
        if (selected == NULL || temp->task->deadline < selected->deadline) {
            selected = temp->task;
        }
        temp = temp->next;
    }
    return selected;
}

// Escalonador EDF
void schedule() {
    pthread_t timer_thread;
    pthread_create(&timer_thread, NULL, timer_function, NULL);

    printf("\n[ESCALONADOR EDF INICIADO]\n");

    while (task_list != NULL) {
        Task *tarefa = select_task_with_earliest_deadline();

        if (tarefa == NULL) {
            printf("[ESCALONADOR] Nenhuma tarefa disponível. Ocioso.\n");
            break;
        }

        printf("[ESCALONADOR] Executando task %s (TID %d) com deadline %d\n",
               tarefa->name, tarefa->tid, tarefa->deadline);

        run(tarefa, tarefa->burst);  // Executa a task inteira

        delete(&task_list, tarefa);  // Remove a task da lista após execução
    }

    rodando = 0;  // Para o timer
    pthread_join(timer_thread, NULL);

    printf("[ESCALONADOR EDF FINALIZADO]\n");
}
