#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "task.h"

// Função auxiliar para criar uma nova Task

int main() {
    struct node *head = NULL;

    // Criar algumas tasks
    Task *t1 = create_task("Processo A", 1, 2, 10, 50, 0);
    Task *t2 = create_task("Processo B", 2, 1, 5, 30, 0);
    Task *t3 = create_task("Processo C", 3, 3, 8, 40, 0);

    // Inserir no final da lista
    insert_at_tail(&head, t1);
    insert_at_tail(&head, t2);

    // Inserir no início da lista
    insert_at_head(&head, t3);

    printf("Lista após inserções:\n");
    traverse_list(head);

    // Remover um processo específico (Processo B)
    printf("\nRemovendo 'Processo B' da lista...\n");
    delete_task(&head, t2);
    free_task(t2); // também liberar a task para evitar vazamento

    traverse_list(head);

    // Remover e obter a primeira task da lista
    printf("\nRemovendo a primeira task da lista...\n");
    Task *removed = remove_first_task(&head);
    if (removed) {
        printf("Task removida: %s\n", removed->name);
        free_task(removed);
    }

    printf("\nLista após remoção da primeira task:\n");
    traverse_list(head);

    // Limpar o restante da lista e liberar memória
    while ((removed = remove_first_task(&head)) != NULL) {
        free_task(removed);
    }

    return 0;
}
