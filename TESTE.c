/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers_edf.h" 
#include "list.h"     
#include "task.h"     
#include "timer.h"     


int main() {
    
/*
    add("Tarefa1", 3, 9);
    add("Tarefa2", 2, 6);
    add("Tarefa3", 1, 15);
    add("Tarefa4", 4, 11);
*/ 
/*
     timer_set_quantum(1);  // 1 tick = 100ms

    // Adiciona tarefas periódicas: nome, prioridade, período, tempo de processamento
    add_task("P1", 1, 50, 25);
    add_task("P2", 1, 75, 30);
    printf("\n--- Starting Scheduling Simulation ---\n");

    // Invoca o escalonador
    schedule();
    
    return 0;
}
*/
#include <stdio.h>
#include <stdlib.h>
#include "schedulers_rr.h"  // ou como for seu header RR
#include "list.h"     
#include "task.h"     
#include "timer.h" 

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_tarefas.txt>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    char line[256];
    char nome[50];
    int prioridade, burst;

    while (fgets(line, sizeof(line), file)) {
        // Ler linhas no formato "nome, prioridade, burst"
        if (sscanf(line, "%[^,], %d, %d", nome, &prioridade, &burst) == 3) {
            add(nome, prioridade, burst);
        } else {
            printf("Linha inválida ou formato incorreto: %s", line);
        }
    }

    fclose(file);

    schedule();  // função que roda o escalonador RR

    return 0;
}
