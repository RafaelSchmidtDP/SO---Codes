#include <stdio.h>
#include "schedulers_edf.h"

// Função principal
int main() {
    printf("\n========================\n");
    printf("Teste Escalonador RR com Prioridade\n");
    printf("========================\n\n");

    add("Tarefa1", 2, 10, 25);  // deadline 25
    add("Tarefa2", 1, 5, 15);   // deadline 15
    add("Tarefa3", 3, 8, 30);   // deadline 30
    add("Tarefa4", 2, 6, 20);   // deadline 20
    add("Tarefa5", 1, 4, 10);   // deadline 10

    printf("\n>>> Iniciando Escalonamento...\n");
    schedule();
    printf("\n>>> Escalonamento concluído.\n");

    return 0;
}
