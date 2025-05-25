#ifndef ESCALONADOR_H
#define ESCALONADOR_H

#define N 4 // Número de processos (pode ajustar)
#define LIMIAR_AGING 3 // Limiar de envelhecimento
#include "task.h"
// Funções do escalonador
void aging(Task processos[], int n, int exec_pid);
int proximo_processo(Task processos[], int n);
int todos_finalizados(Task processos[], int n);

#endif