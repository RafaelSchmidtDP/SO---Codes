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
Task *executando = NULL;

// Adiciona task na lista
void add(char *name, int priority, int burst, int deadline, int periodo) {
    static int tid_counter = 1;
    Task *task = create_task(name, tid_counter++, priority, burst, deadline, periodo);
    task->remaining = burst;
    task->next_release = deadline; // deadline inicial é o primeiro release
    task->release_count = 0; // Contador de quantas vezes ela foi liberada
    insert_at_tail(&task_list, task);
}

// Seleciona task com menor deadline
Task *select_task_with_earliest_deadline() {
    struct node *temp = task_list;
    Task *selected = NULL;

    while (temp != NULL) {
        Task *t = temp->task;
        if (t->remaining > 0) {
            if (selected == NULL || t->deadline < selected->deadline) {
                selected = t;
            }
        }
        temp = temp->next;
    }
    return selected;
}

// Atualiza tarefas periódicas no tempo atual
void atualiza_periodicidade() {
    struct node *temp = task_list;

    while (temp != NULL) {
        Task *tarefa = temp->task;

        if (tempo_atual == tarefa->next_release) {
            tarefa->remaining = tarefa->burst;
            tarefa->deadline = tarefa->next_release;
            tarefa->next_release += tarefa->periodo;
            tarefa->release_count++;

            printf("[RELEASE] Tarefa %s liberada. Deadline: %d (Release count: %d)\n",
                   tarefa->name, tarefa->deadline, tarefa->release_count);
        }

        temp = temp->next;
    }
}

// Verifica se todas as tarefas atingiram 2 deadlines
int todas_tarefas_completaram() {
    struct node *temp = task_list;

    while (temp != NULL) {
        if (temp->task->release_count < 2) {
            return 0;
        }
        temp = temp->next;
    }
    return 1;
}

// Escalonador EDF periódico e preemptivo
void schedule() {
    printf("\n[ESCALONADOR EDF PERIODICO E PREEMPTIVO INICIADO]\n");

    while (rodando) {
        atualiza_periodicidade();

        if (todas_tarefas_completaram()) {
            printf("[ESCALONADOR] Todas as tarefas completaram 2 deadlines. Encerrando.\n");
            break;
        }

        Task *proxima = select_task_with_earliest_deadline();

        if (proxima == NULL) {
            printf("[ESCALONADOR] Nenhuma tarefa disponível. Ocioso.\n");
            sleep(1);
            tempo_atual++;
            continue;
        }

        // Preempção se necessário
        if (executando == NULL || proxima->deadline < executando->deadline) {
            executando = proxima;
            printf("[ESCALONADOR] Executando task %s (TID %d) com deadline %d (Remaining: %d)\n",
                   executando->name, executando->tid, executando->deadline, executando->remaining);
        }

        // Executa 1 unidade de tempo
        run(executando, 1);
        executando->remaining--;

        // Verifica deadline perdido
        if (tempo_atual >= executando->deadline && executando->remaining > 0) {
            printf("[ALERTA] Task %s perdeu o deadline! (Deadline: %d, Tempo atual: %d)\n",
                   executando->name, executando->deadline, tempo_atual);
            executando->remaining = 0; // Considera como falha, zera a execução
        }

        // Se terminou, libera
        if (executando->remaining == 0) {
            printf("[ESCALONADOR] Tarefa %s terminou sua capacidade.\n", executando->name);
            executando = NULL;
        }

        sleep(1);
        tempo_atual++;
    }

    printf("[ESCALONADOR EDF FINALIZADO]\n");
}
