// schedule_rr_p.h
#ifndef SCHEDULE_RR_P_H // Boa prática para o nome da guarda de inclusão
#define SCHEDULE_RR_P_H

#include "task.h" // Inclua task.h, pois Task será usada internamente no .c

#define MIN_PRIORITY 1
#define MAX_PRIORITY 10

// insere nova tarefa
// A assinatura deve refletir os dados que vêm do driver.c
// e que serão usados para criar a Task.
void add(char *name, int priority, int burst);

// faz o escalonamento de acordo com RR_p
void schedule();

#endif // SCHEDULE_RR_P_H