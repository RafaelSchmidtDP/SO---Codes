#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"

struct node *task_list = NULL;
int tempo_atual = 0;
int rodando = 1;

// Timer que simula o clock
void *timer_function(void *arg) {
    while (rodando) {
        sleep(1);
        tempo_atual++;
        printf("[TIMER] Tempo atual: %d\n", tempo_atual);
    }
    return NULL;
}

// Adiciona task na lista
void add(char *name, int priority, int burst, int deadline) {
    static int tid_counter = 1;
    Task *task = create_task(name, tid_counter++, priority, burst, deadline, 0);
    insert_at_tail(&task_list, task);
}

// Escolhe a task com menor deadline (relativo ao tempo atual)
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

// EDF sem preempção (executa até terminar)
void schedule() {
    pthread_t timer_thread;
    pthread_create(&timer_thread, NULL, timer_function, NULL);

    printf("\n[ESCALONADOR EDF INICIADO]\n");

    while (task_list != NULL) {
        Task *tarefa = select_task_with_earliest_deadline();

        if (tarefa == NULL) {
            printf("[ESCALONADOR] Nenhuma tarefa disponível. Ocioso.\n");
            sleep(1);
            tempo_atual++;
            continue;
        }

        printf("[ESCALONADOR] Executando task %s (TID %d) com deadline %d\n",
               tarefa->name, tarefa->tid, tarefa->deadline);

        run(tarefa, tarefa->burst);

        if (tempo_atual > tarefa->deadline) {
            printf("[ALERTA] Task %s perdeu o deadline! (Deadline: %d, Tempo final: %d)\n",
                   tarefa->name, tarefa->deadline, tempo_atual);
        }

        delete_task(&task_list, tarefa);
        free_task(tarefa);
    }

    rodando = 0;
    pthread_join(timer_thread, NULL);

    printf("[ESCALONADOR EDF FINALIZADO]\n");
}
