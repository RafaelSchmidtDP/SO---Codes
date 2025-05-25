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

// Adiciona task na lista (tarefa periódica)
void add(char *name, int priority, int burst, int deadline, int periodo) {
    static int tid_counter = 1;
    Task *task = create_task(name, tid_counter++, priority, burst, deadline, periodo);
    insert_at_tail(&task_list, task);
}

// Escolhe a task com menor deadline relativo
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

// Função para atualizar deadlines e adicionar instâncias periódicas
void atualiza_periodicidade() {
    struct node *temp = task_list;

    while (temp != NULL) {
        Task *tarefa = temp->task;

        if (tempo_atual >= tarefa->next_release) {
            if (tarefa->remaining == 0) {
                // Reset da tarefa periódica
                tarefa->remaining = tarefa->burst;
                tarefa->deadline = tempo_atual + tarefa->periodo;
                tarefa->next_release += tarefa->periodo;
                printf("[RELEASE] Tarefa %s liberada. Novo deadline: %d\n", tarefa->name, tarefa->deadline);
            }
        }

        temp = temp->next;
    }
}

// Escalonador EDF preemptivo e periódico
void schedule() {
    pthread_t timer_thread;
    pthread_create(&timer_thread, NULL, timer_function, NULL);

    printf("\n[ESCALONADOR EDF PERIODICO E PREEMPTIVO INICIADO]\n");
,
    Task *executando = NULL;

    while (rodando) {
        atualiza_periodicidade();

        Task *proxima = select_task_with_earliest_deadline();

        if (proxima == NULL) {
            printf("[ESCALONADOR] Nenhuma tarefa disponível. Ocioso.\n");
            sleep(1);
            tempo_atual++;
            continue;
        }

        // Preempção: troca se a próxima tem deadline menor que a atual
        if (executando == NULL || proxima->deadline < executando->deadline) {
            executando = proxima;
            printf("[ESCALONADOR] Executando task %s (TID %d) com deadline %d\n",
                   executando->name, executando->tid, executando->deadline);
        }

        // Executa 1 unidade de tempo
        run(executando, 1);
        executando->remaining--;

        // Verifica deadline perdido
        if (tempo_atual > executando->deadline && executando->remaining > 0) {
            printf("[ALERTA] Task %s perdeu o deadline! (Deadline: %d, Tempo atual: %d)\n",
                   executando->name, executando->deadline, tempo_atual);
        }

        // Se terminou, fica aguardando próxima liberação
        if (executando->remaining == 0) {
            printf("[ESCALONADOR] Tarefa %s terminou essa instância.\n", executando->name);
            executando = NULL;
        }

        sleep(1);
        tempo_atual++;
    }

    pthread_join(timer_thread, NULL);

    printf("[ESCALONADOR EDF FINALIZADO]\n");
}
