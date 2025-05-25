// schedule_rr.h
#ifndef SCHEDULE_RR_H
#define SCHEDULE_RR_H

#include "task.h" // Inclua task.h para a estrutura Task

// Definindo os limites de prioridade.
// Mantidos aqui para consistência com outros escalonadores,
// embora a prioridade não seja diretamente utilizada para a lógica de escalonamento RR.
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10

// Declaração da função para adicionar uma tarefa à lista de aptos para o RR clássico.
// Recebe nome, prioridade (será armazenada, mas ignorada para decisão de escalonamento RR)
// e tempo de burst.
void add(char *name, int priority, int burst);

// Declaração da função principal do escalonador Round-Robin clássico.
// Esta função irá invocar a lógica de escalonamento.
void schedule();

#endif // SCHEDULE_RR_H