#ifndef SCHEDULE_EDF_H
#define SCHEDULE_EDF_H

#include "task.h" // Garante que a struct Task está disponível

// Define o tempo máximo de simulação em unidades de tempo
#define MAX_SIMULATION_TIME 300 // Exemplo: simular até o tempo 300

// Array global para a timeline de saída (0 para CPU ociosa, TID da tarefa para execução)
// Será preenchido pela thread de escalonamento.
extern int timeline_output[MAX_SIMULATION_TIME];

// Flag para sinalizar que a simulação deve ser encerrada
extern int simulation_completed_flag;

// Adiciona uma definição de tarefa original (para que possamos gerenciar instâncias periódicas)
void add(char *name, int priority, int burst, int deadline, int periodo, int arrival_time);

// Invoca o escalonador principal
void schedule();

#endif // SCHEDULE_EDF_H
