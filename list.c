// list.c - VERSÃO CORRIGIDA
// Deve conter APENAS as definições da struct node, Task, e funções de lista/tarefa.

#include <stdlib.h>   // Para malloc, free, exit
#include <stdio.h>    // Para printf, perror
#include <string.h>   // Para funções de string (se usadas em outros arquivos)

#include "list.h"     // Inclui definições da lista (node)
#include "task.h"     // Inclui definição da struct Task

// Libera a memória de uma tarefa
void free_task(Task *task) {
    if (task) {                      // Verifica se o ponteiro não é NULL
        free(task->name);             // Libera memória alocada para o nome da tarefa
        free(task);                   // Libera a estrutura da tarefa
    }
}

// Insere uma tarefa no final da lista
void insert_at_tail(struct node **head, Task *newTask) {
    struct node *newNode = (struct node*) malloc(sizeof(struct node)); // Aloca memória para um novo nó
    if (newNode == NULL) {                                              // Verifica se a alocação falhou
        perror("Erro ao alocar memória para o nó");                    // Imprime erro
        exit(EXIT_FAILURE);                                             // Encerra o programa
    }
    newNode->task = newTask;        // Atribui a tarefa ao novo nó
    newNode->next = NULL;           // O próximo é NULL porque será o último

    if (*head == NULL) {            // Se a lista está vazia
        *head = newNode;             // O novo nó vira a cabeça da lista
    } else {
        struct node *temp = *head;  // Cria um ponteiro temporário para percorrer
        while (temp->next != NULL) {// Percorre até o último nó
            temp = temp->next;
        }
        temp->next = newNode;       // Liga o último nó ao novo nó
    }
}

// Remove uma tarefa específica da lista
void delete_task(struct node **head, Task *task) {
    if (*head == NULL) {            // Lista vazia, nada a fazer
        return;
    }

    struct node *temp = *head;      // Ponteiro para percorrer a lista
    struct node *prev = NULL;       // Ponteiro para o nó anterior

    if (temp->task == task) {       // Se a tarefa está no primeiro nó
        *head = temp->next;         // Atualiza a cabeça da lista
        free(temp);                 // Libera o nó
        return;
    }

    prev = temp;                    // Inicializa o nó anterior
    temp = temp->next;              // Começa do segundo nó

    // Procura a tarefa na lista
    while (temp != NULL && temp->task != task) {
        prev = temp;                // Avança o anterior
        temp = temp->next;          // Avança o atual
    }

    if (temp != NULL) {             // Se encontrou a tarefa
        prev->next = temp->next;    // Remove o nó da lista
        free(temp);                 // Libera o nó
    }
}

// Remove o primeiro nó da lista e retorna a tarefa contida nele
Task* remove_first_task(struct node **head) {
    if (*head == NULL) {            // Lista vazia
        return NULL;
    }
    struct node *temp = *head;      // Guarda o nó da cabeça
    Task *task = temp->task;        // Guarda a tarefa do nó
    *head = temp->next;             // Atualiza a cabeça da lista
    free(temp);                     // Libera o nó antigo
    return task;                    // Retorna a tarefa
}

// Percorre a lista e imprime as informações das tarefas
void traverse_list(struct node *head) {
    if (head == NULL) {             // Se lista vazia
        printf("Lista vazia.\n");   // Imprime que está vazia
        return;
    }
    struct node *temp = head;       // Ponteiro para percorrer a lista
    while (temp != NULL) {          // Enquanto não chegar no final
        printf("[%s] [%d] [%d] [%d] [%d]\n", // Imprime dados da tarefa
               temp->task->name,
               temp->task->priority,
               temp->task->burst,
               temp->task->deadline,
               temp->task->waiting_time);
        temp = temp->next;          // Avança para o próximo nó
    }
}
