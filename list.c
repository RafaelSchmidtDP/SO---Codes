// list.c - VERSÃO CORRIGIDA
// Deve conter APENAS as definições da struct node, Task, e funções de lista/tarefa.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h" 
#include "task.h" // Inclui o cabeçalho da tarefa para a definição da struct Task

// Variável global estática para gerar IDs de tarefa únicos
// Corretamente definida aqui se create_task está em list.c
static int tid_counter = 0;

/**
 * @brief Cria uma nova estrutura Task e a inicializa.
 * O Task ID (tid) é gerado automaticamente.
 * @param name Nome da tarefa.
 * @param priority Prioridade da tarefa.
 * @param burst Tempo de execução total da tarefa.
 * @param deadline Prazo final da tarefa.
 * @param periodo Período da tarefa (para tarefas periódicas).
 * @return Um ponteiro para a nova Task criada.
 */
Task *create_task(char *name, int priority, int burst, int deadline, int periodo) {
    Task *task = (Task *)malloc(sizeof(Task));
    if (task == NULL) {
        perror("Falha ao alocar memória para a tarefa");
        exit(EXIT_FAILURE);
    }
    task->name = strdup(name);
    task->tid = ++tid_counter; // Atribui um TID único
    task->priority = priority;
    task->burst = burst;
    task->deadline = deadline;
    task->periodo = periodo;

    task->arrival = 0; // Será definido quando a tarefa for adicionada à fila de prontos ou "chegar"
    task->remaining = burst; // Inicialmente, o tempo restante é o burst total
    task->next_release = 0;  // Para tarefas periódicas, a próxima vez que estará pronta
    task->release_count = 0; // Quantas vezes foi liberada

    task->waiting_time = 0; // Inicializa o tempo de espera
    return task;
}

/**
 * @brief Libera a memória alocada para uma estrutura Task.
 * @param task Ponteiro para a Task a ser liberada.
 */
void free_task(Task *task) {
    if (task) {
        free(task->name);
        free(task);
    }
}

/**
 * @brief Insere um novo nó no início da lista (comportamento de pilha).
 * @param head Ponteiro para o ponteiro da cabeça da lista.
 * @param newTask Ponteiro para a Task a ser inserida.
 */
void insert_at_head(struct node **head, Task *newTask) {
    struct node *newNode = (struct node*) malloc(sizeof(struct node));
    if (newNode == NULL) {
        perror("Erro ao alocar memória para o nó");
        exit(EXIT_FAILURE);
    }
    newNode->task = newTask;
    newNode->next = *head; // O novo nó aponta para a antiga cabeça
    *head = newNode;       // A cabeça da lista agora é o novo nó
}

/**
 * @brief Insere um novo nó no final da lista (comportamento FIFO).
 * @param head Ponteiro para o ponteiro da cabeça da lista.
 * @param newTask Ponteiro para a Task a ser inserida.
 */
void insert_at_tail(struct node **head, Task *newTask) {
    struct node *newNode = (struct node*) malloc(sizeof(struct node));
    if (newNode == NULL) {
        perror("Erro ao alocar memória para o nó");
        exit(EXIT_FAILURE);
    }
    newNode->task = newTask;
    newNode->next = NULL; // Novo nó sempre será o último

    if (*head == NULL) {
        // Se a lista está vazia, o novo nó se torna a cabeça
        *head = newNode;
    } else {
        // Percorre até o final da lista
        struct node *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode; // Adiciona o novo nó no final
    }
}

/**
 * @brief Insere um novo nó na lista, mantendo a ordem por deadline (Earliest Deadline First).
 * A tarefa com o menor deadline fica no início da lista.
 * @param head Ponteiro para o ponteiro da cabeça da lista.
 * @param newTask Ponteiro para a Task a ser inserida.
 */
void insert_sorted_by_deadline(struct node **head, Task *newTask) {
    struct node *newNode = (struct node*) malloc(sizeof(struct node));
    if (newNode == NULL) {
        perror("Falha ao alocar memória para o novo nó");
        exit(EXIT_FAILURE);
    }
    newNode->task = newTask;
    newNode->next = NULL;

    // Se a lista está vazia ou a nova tarefa tem um deadline anterior ao da cabeça
    if (*head == NULL || newTask->deadline < (*head)->task->deadline) {
        newNode->next = *head;
        *head = newNode;
    } else {
        struct node *current = *head;
        // Percorre a lista para encontrar o ponto de inserção correto
        while (current->next != NULL && newTask->deadline >= current->next->task->deadline) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

/**
 * @brief Deleta a tarefa selecionada da lista.
 * @param head Ponteiro para o ponteiro da cabeça da lista.
 * @param task Ponteiro para a Task a ser deletada.
 */
void delete_task(struct node **head, Task *task) {
    if (*head == NULL) {
        return; // Lista vazia
    }

    struct node *temp = *head;
    struct node *prev = NULL;

    // Caso especial: a tarefa a ser deletada é a primeira da lista
    if (temp->task == task) { // Compara diretamente os ponteiros da tarefa
        *head = temp->next; // A cabeça da lista avança
        free(temp);         // Libera a memória do nó
        return;
    }

    // Procura a tarefa na lista
    prev = temp;
    temp = temp->next;
    while (temp != NULL && temp->task != task) { // Compara diretamente os ponteiros da tarefa
        prev = temp;
        temp = temp->next;
    }

    // Se a tarefa foi encontrada
    if (temp != NULL) {
        prev->next = temp->next; // O nó anterior aponta para o próximo do nó a ser deletado
        free(temp);             // Libera a memória do nó
    }
    // else: tarefa não encontrada na lista, não faz nada
}

/**
 * @brief Pega a primeira tarefa da lista sem removê-la.
 * @param head Ponteiro para a cabeça da lista.
 * @return Ponteiro para a primeira Task na lista, ou NULL se a lista estiver vazia.
 */
Task* get_first_task(struct node *head) {
    if (head == NULL) {
        return NULL; // Lista vazia
    }
    return head->task;
}

Task* remove_first_task(struct node **head) {
    if (*head == NULL) {
        return NULL; // Lista vazia
    }
    struct node *temp = *head; // Guarda o nó da cabeça
    Task *task = temp->task;   // Pega a tarefa do nó da cabeça
    *head = temp->next;        // A cabeça da lista avança
    free(temp);                // Libera a memória do nó removido
    return task;               // Retorna a tarefa
}

// Percorre e imprime os elementos da lista
void traverse_list(struct node *head) {
    if (head == NULL) {
        printf("Lista vazia.\n");
        return;
    }
    struct node *temp = head;
    while (temp != NULL) {
        printf("[%s] [%d] [%d] [%d] [%d]\n",
               temp->task->name,
               temp->task->priority,
               temp->task->burst,
               temp->task->deadline,
               temp->task->waiting_time);
        temp = temp->next;
    }
}