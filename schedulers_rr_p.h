// schedulers_rr_p.h - Round-Robin com Prioridade
#ifndef SCHEDULERS_RR_P_H
#define SCHEDULERS_RR_P_H

#include "task.h"

// insere nova tarefa
void add(Task *t);

// faz o escalonamento de acordo com RR_p
void schedule();

#endif // SCHEDULERS_RR_P_H
