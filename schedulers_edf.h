// schedulers_edf.h - Earliest Deadline First
#ifndef SCHEDULERS_EDF_H
#define SCHEDULERS_EDF_H

#include "task.h"

// insere nova tarefa
void add(Task *t);

// faz o escalonamento de acordo com EDF
void schedule();

#endif // SCHEDULERS_EDF_H
