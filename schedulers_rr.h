// schedulers_rr.h - Round-Robin clássico
#ifndef SCHEDULERS_RR_H
#define SCHEDULERS_RR_H

#include "task.h"

// insere nova tarefa
void add(Task *t);

// faz o escalonamento de acordo com RR
void schedule();

#endif // SCHEDULERS_RR_H
