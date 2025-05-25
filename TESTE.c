#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers_edf.h" 
#include "list.h"     // Inclua list.h para ter acesso a Task* e struct node
#include "task.h"     // Inclua task.h para a definição de Task

// Certifique-se de que MAX_SIMULATION_TIME está definido em schedulers_edf.h ou um arquivo de config
// Exemplo: #define MAX_SIMULATION_TIME 1000 

int main() {
    // Definir as tarefas diretamente como no seu exemplo
    // Tarefa P1: Exec: 25, Period: 50, Deadline: 50, Arrival: 0
    // Tarefa P2: Exec: 30, Period: 75, Deadline: 75, Arrival: 0 (corrigida para deadline = 75)

    printf("\n--- Leitura das Tarefas ---\n");

    // Adiciona a definição da Tarefa P1
    // add_original_task_definition(name, priority, burst, deadline_relative, periodo, arrival_time)
    add_original_task_definition(strdup("P1"), 1, 25, 50, 50, 0); 
    
    // Adiciona a definição da Tarefa P2
    // IMPORTANTE: Usei Deadline 75 aqui para replicar o comportamento escalonável do seu segundo exemplo.
    // Se você *realmente* quiser a deadline 30 para P2 (como no seu primeiro log), a simulação
    // *irá* mostrar perdas de prazo porque o conjunto não é escalonável com essa deadline apertada.
    add_original_task_definition(strdup("P2"), 2, 30, 75, 75, 0); 

    printf("\n--- Conjunto de Tarefas Definidas ---\n");
    // (Você pode adicionar um loop para imprimir a lista de definições originais
    // se quiser que ela apareça aqui, mas não é estritamente necessário para o escalonador funcionar.)

    // Para calcular e exibir a utilização (como no seu segundo exemplo de saída)
    double utilization = (double)25 / 50 + (double)30 / 75;
    printf("\nTotal Utilization: %.2lf\n", utilization);
    if (utilization > 1.0) {
        printf("Warning: Task set is not feasible (utilization > 1)\n");
    }

    printf("\n--- Starting Scheduling Simulation ---\n");

    // Invoca o escalonador
    schedule();

    return 0;
}