#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"
#include "CPU.h"
#include "timer.h"
#include "schedulers_edf.h"

struct node *fila_tarefas = NULL;
int tid_counter = 1;

// Adiciona tarefa na fila EDF
void add(char *name, int priority, int burst, int deadline) {
    printf("Adicionando tarefa: %s, Priority: %d, Burst: %d, Deadline: %d\n",
           name, priority, burst, deadline);
    Task *task = (Task *)malloc(sizeof(Task));

    task->name = strdup(name);
    task->priority = priority;
    task->burst = burst;
    task->deadline = deadline;
    task->waiting_time = 0;
    task->tid = tid_counter++;

    insert_at_tail(&fila_tarefas, task);
}

// Busca a tarefa com menor deadline
Task *buscar_menor_deadline(struct node *head) {
    if (head == NULL) {
        return NULL;
    }

    struct node *atual = head;
    Task *tarefa_menor = atual->task;

    while (atual != NULL) {
        if (atual->task->deadline < tarefa_menor->deadline) {
            tarefa_menor = atual->task;
        }
        atual = atual->next;
    }

    return tarefa_menor;
}

// Loop principal do EDF
void schedule() {
    timer_set_quantum(1);
    timer_start();

    printf("\n[EDF] Iniciando escalonamento...\n");

    while (fila_tarefas != NULL) {
        Task *tarefa = buscar_menor_deadline(fila_tarefas);

        if (tarefa == NULL) {
            break;
        }

        printf("[EDF] Executando tarefa: %s (TID: %d, Deadline: %d, Burst: %d)\n",
               tarefa->name, tarefa->tid, tarefa->deadline, tarefa->burst);

        for (int i = 0; i < tarefa->burst; i++) {
            run(tarefa, 1);
            timer_wait_quantum_expired();

            int tempo_atual = get_global_time();
            /*if (tempo_atual > tarefa->deadline) {
                printf("!!! ALERTA: Tarefa %s (TID: %d) perdeu o deadline! Tempo atual: %d, Deadline: %d\n",
                       tarefa->name, tarefa->tid, tempo_atual, tarefa->deadline);
                // Se quiser, pode decidir interromper a execução da tarefa aqui
                // break; // descomente se quiser interromper a tarefa quando perder deadline
            }
                */
        }

        printf("[EDF] Tarefa %s (TID: %d) concluída no tempo %d\n",
               tarefa->name, tarefa->tid, get_global_time());

        delete_task(&fila_tarefas, tarefa);
        free_task(tarefa);
    }

    printf("[EDF] Todas as tarefas foram concluídas.\n");
    timer_stop();
}
