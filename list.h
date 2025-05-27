/**
 * Várias operações de lista usando gerenciamento direto do ponteiro de cabeça
 */

#ifndef LIST_H
#define LIST_H

#include "task.h" // Inclui a estrutura Task

// Definição da estrutura do nó da lista encadeada
struct node {
    Task *task;
    struct node *next;
};

// Funções de operação na lista.
// Todas agora recebem struct node **head para modificar a cabeça da lista.
void insert_at_tail(struct node **head, Task *task); // Para comportamento FIFO (RR clássico)
void delete_task(struct node **head, Task *task); // Deleta uma tarefa específica
Task* get_first_task(struct node *head); // Pega a primeira tarefa sem remover
Task* remove_first_task(struct node **head); // Remove e retorna a primeira tarefa
void traverse_list(struct node *head); // Percorre e imprime a lista

//liberação de tarefas (geralmente em task.c/h, mas incluídas aqui para referência)
void free_task(Task *task); // Libera a memória alocada para uma tarefa

#endif // LIST_H
