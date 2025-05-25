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
void insert_at_head(struct node **head, Task *task); // Para o comportamento de pilha original
void insert_at_tail(struct node **head, Task *task); // Para comportamento FIFO (RR clássico)
void insert_sorted_by_deadline(struct node **head, Task *newTask); // Para EDF (Earliest Deadline First)
void delete_task(struct node **head, Task *task); // Deleta uma tarefa específica
Task* get_first_task(struct node *head); // Pega a primeira tarefa sem remover
Task* remove_first_task(struct node **head); // Remove e retorna a primeira tarefa
void traverse_list(struct node *head); // Percorre e imprime a lista

// Funções de criação e liberação de tarefas (geralmente em task.c/h, mas incluídas aqui para referência)
Task *create_task(char *name, int priority, int burst, int deadline, int periodo);
void free_task(Task *task); // Libera a memória alocada para uma tarefa

#endif // LIST_H
