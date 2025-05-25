#include <stdio.h>
#include "schedulers_edf.h"

// Função principal
int main() {
    printf("\n========================\n");
    printf("Teste Escalonador RR com Prioridade\n");
    printf("========================\n\n");

    add("Tarefa1", 1, 2, 4, 4); // periodo 4, capacidade 2, deadline 4
    add("Tarefa2", 1, 1, 5, 5); // periodo 5, capacidade 1, deadline 5
    add("Tarefa3", 1, 2, 8, 8); // periodo 8, capacidade 2, deadline 8

    printf("\n>>> Iniciando Escalonamento...\n");
    schedule();
    printf("\n>>> Escalonamento concluído.\n");

    return 0;
}
