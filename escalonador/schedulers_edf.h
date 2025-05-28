#ifndef EDF_H
#define EDF_H

#include "list.h"
#include "task.h"
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10

void add(char *name, int priority, int burst, int deadline);
// Executa o escalonador EDF
void schedule();

#endif
