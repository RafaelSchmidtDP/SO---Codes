#ifndef AGING_H
#define AGING_H
#define MAX_PRIORITY 10
#define MIN_PRIORITY 1

void add(char *name, int priority, int burst);
void schedule();
void imprimir_filas_e_execucao(Task *executando);
Task *proximo_processo(); // ver se pode ficar aqui 
#endif // AGING_H