#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // Para usleep
#include <string.h> // Para strdup

#include "list.h" 
#include "task.h" 
#include "schedulers_edf.h" 

// =========================================================
// VARIÁVEIS GLOBAIS DO ESCALONADOR
// =========================================================

static struct node *task_list_head = NULL;
static struct node *original_tasks_definitions_list = NULL;
static int current_time = 0;
static pthread_mutex_t list_mutex;
static pthread_cond_t timer_cond;
static int last_processed_time = -1;
int timeline_output[MAX_SIMULATION_TIME];
int simulation_completed_flag = 0;

// =========================================================
// FUNÇÕES AUXILIARES E THREADS
// =========================================================

void *timer_thread(void *arg) {
    while (current_time < MAX_SIMULATION_TIME) {
        usleep(100000); 

        pthread_mutex_lock(&list_mutex);
        current_time++;
        pthread_cond_signal(&timer_cond); 
        pthread_mutex_unlock(&list_mutex);
    }
    simulation_completed_flag = 1;
    pthread_cond_signal(&timer_cond);
    return NULL;
}

/**
 * @brief Adiciona uma nova definição de tarefa ao sistema.
 * @param name Nome da tarefa.
 * @param priority Prioridade da tarefa (não usada diretamente no EDF para ordenação).
 * @param burst Tempo de execução total necessário para cada instância.
 * @param deadline_relative Prazo final relativo para a instância.
 * @param periodo Período da tarefa.
 * @param arrival_time Tempo em que a primeira instância da tarefa chega.
 */
void add_original_task_definition(char *name, int priority, int burst, int deadline_relative, int periodo, int arrival_time) {
    // Cria uma "task definition" - isto é, um objeto Task que guarda as propriedades originais.
    // O 'deadline_relative' é o valor original D.
    Task *new_task_def = create_task(name, priority, burst, deadline_relative, periodo);
    new_task_def->arrival = arrival_time;
    new_task_def->next_release = arrival_time;

    pthread_mutex_lock(&list_mutex);
    insert_at_tail(&original_tasks_definitions_list, new_task_def); 

    // Se a primeira instância chega no tempo 0, já a adiciona à fila de prontos.
    if (new_task_def->arrival == 0) {
        // Cria uma CÓPIA para a primeira instância na fila de prontos
        Task *first_instance = create_task(new_task_def->name, new_task_def->priority, new_task_def->burst,
                                            new_task_def->deadline, new_task_def->periodo);
        first_instance->arrival = 0; // Chegada da primeira instância
        // A deadline ABSOLUTA para esta instância é o tempo atual (0) + o deadline RELATIVO
        first_instance->deadline = first_instance->arrival + new_task_def->deadline; 
        first_instance->remaining = new_task_def->burst;
        
        insert_sorted_by_deadline(&task_list_head, first_instance);
    }
    pthread_mutex_unlock(&list_mutex);

    printf("Adicionada definicao de tarefa: %s (Burst: %d, Periodo: %d, Deadline: %d, Chegada: %d)\n",
           name, burst, periodo, deadline_relative, arrival_time); // Imprime a deadline relativa
}

// =========================================================
// ESCALONADOR EDF
// =========================================================

void schedule() {
    pthread_t timer_tid;
    pthread_mutex_init(&list_mutex, NULL);
    pthread_cond_init(&timer_cond, NULL);

    for (int i = 0; i < MAX_SIMULATION_TIME; i++) {
        timeline_output[i] = 0;
    }

    if (pthread_create(&timer_tid, NULL, timer_thread, NULL) != 0) {
        perror("Falha ao criar thread do timer");
        exit(EXIT_FAILURE);
    }

    printf("Iniciando o Escalonador EDF (simulacao ate o Tempo %d)...\n", MAX_SIMULATION_TIME);

    Task *current_running_task = NULL;

    while (current_time < MAX_SIMULATION_TIME || task_list_head != NULL) {
        pthread_mutex_lock(&list_mutex);

        while (last_processed_time >= current_time && !simulation_completed_flag) {
            pthread_cond_wait(&timer_cond, &list_mutex);
        }

        if (simulation_completed_flag && task_list_head == NULL && current_running_task == NULL) {
            pthread_mutex_unlock(&list_mutex);
            break;
        }

        if (current_time >= MAX_SIMULATION_TIME) {
            pthread_mutex_unlock(&list_mutex);
            continue;
        }

        last_processed_time = current_time;

        // --- Lógica de Liberação de Tarefas Periódicas ---
        struct node *current_def_node = original_tasks_definitions_list;
        while (current_def_node != NULL) {
            Task *task_def = (Task *)current_def_node->task;

            // Verifica se o tempo atual é um ponto de liberação para uma nova instância
            // E se ainda não foi liberada para este 'current_time' (para evitar duplicatas)
            if (current_time >= task_def->arrival && 
                (task_def->periodo == 0 ? (current_time == task_def->arrival) : ((current_time - task_def->arrival) % task_def->periodo == 0))) {
                
                // IMPORTANTE: Verifica se uma instância desta tarefa já está ativa E foi liberada neste tempo
                // Isso ajuda a evitar o "duplicação" de uma instância se o escalonador estiver lento
                // OU se a instância anterior ainda não terminou (o que indica um problema de escalonabilidade)
                int instance_already_active_at_this_release = 0;
                struct node *temp_list_node = task_list_head;
                while(temp_list_node != NULL) {
                    // Para identificar se é a mesma *definição* de tarefa e se a chegada coincide com o tempo atual
                    // tid_counter pode ser diferente para instâncias da mesma definição, então use o nome ou propriedades originais
                    // Ou, idealmente, um contador de instância para cada definição
                    if (strcmp(temp_list_node->task->name, task_def->name) == 0 && temp_list_node->task->arrival == current_time) {
                        instance_already_active_at_this_release = 1;
                        break;
                    }
                    temp_list_node = temp_list_node->next;
                }

                if (!instance_already_active_at_this_release) {
                    // Cria uma NOVA instância da tarefa para esta liberação.
                    Task *new_instance = create_task(task_def->name, task_def->priority, task_def->burst,
                                                     task_def->deadline, task_def->periodo); // Use a deadline RELATIVA da definição
                    new_instance->arrival = current_time; // A chegada desta instância é o tempo atual
                    // A deadline ABSOLUTA para esta instância é o tempo de chegada + o deadline RELATIVO
                    new_instance->deadline = new_instance->arrival + task_def->deadline; 
                    new_instance->remaining = task_def->burst;
                    
                    insert_sorted_by_deadline(&task_list_head, new_instance);
                    // printf("DEBUG: Tempo %d: Nova instancia de %s (TID %d) liberada. Deadline: %d\n",
                    //         current_time, new_instance->name, new_instance->tid, new_instance->deadline);
                }
            }
            current_def_node = current_def_node->next;
        }

        // --- Lógica de Escalonamento EDF ---
        Task *next_task_to_run = NULL;
        if (task_list_head != NULL) {
            next_task_to_run = get_first_task(task_list_head);

            if (next_task_to_run != NULL && current_time > next_task_to_run->deadline) {
                printf("Tempo %d: ATENÇÃO! Tarefa %s (TID: %d) PERDEU O PRAZO! Deadline: %d\n",
                       current_time, next_task_to_run->name, next_task_to_run->tid, next_task_to_run->deadline);
            }
        }

        int task_id_for_timeline = 0;
        if (next_task_to_run != NULL && next_task_to_run->remaining > 0) {
            if (current_running_task != next_task_to_run) {
                current_running_task = next_task_to_run;
            }
            current_running_task->remaining--;
            task_id_for_timeline = current_running_task->tid;
        } else {
            current_running_task = NULL;
            task_id_for_timeline = 0;
        }

        if (current_time < MAX_SIMULATION_TIME) {
            timeline_output[current_time] = task_id_for_timeline;
        }

        if (current_running_task != NULL && current_running_task->remaining == 0) {
            delete_task(&task_list_head, current_running_task);
            free_task(current_running_task);
            current_running_task = NULL;
        }
        
        pthread_mutex_unlock(&list_mutex);
    }

    printf("\n--- Timeline (Formato: InicioTempo FimTempo [TaskID]) ---\n");
    int start_time_segment = 0;
    int previous_task_id = -1;

    for (int i = 0; i < MAX_SIMULATION_TIME; i++) {
        int current_task_id = timeline_output[i];

        if (i == 0) {
            previous_task_id = current_task_id;
            start_time_segment = 0;
        } else if (current_task_id != previous_task_id) {
            printf("%4d %4d [Task %d]\n", start_time_segment, i, previous_task_id);
            start_time_segment = i;
            previous_task_id = current_task_id;
        }
    }
    printf("%4d %4d [Task %d]\n", start_time_segment, MAX_SIMULATION_TIME, previous_task_id);

    printf("\nSimulacao finalizada apos %d unidades de tempo.\n", MAX_SIMULATION_TIME);

    pthread_join(timer_tid, NULL);
    pthread_mutex_destroy(&list_mutex);
    pthread_cond_destroy(&timer_cond);

    struct node *current_node = original_tasks_definitions_list;
    while (current_node != NULL) {
        struct node *next_node = current_node->next;
        free_task(current_node->task);
        free(current_node);
        current_node = next_node;
    }
    original_tasks_definitions_list = NULL;

    current_node = task_list_head;
    while (current_node != NULL) {
        struct node *next_node = current_node->next;
        free_task(current_node->task);
        free(current_node);
        current_node = next_node;
    }
    task_list_head = NULL;
}