#ifndef EDF_H
#define EDF_H

#include "task.h"
#include "list.h"

// Estrutura para tarefas periódicas
typedef struct {
    Task *task;
    int period;
    int abs_arrival;
    int abs_deadline;
    int remaining_burst;
} PeriodicTask;

// Extern para acessar na main se quiser
extern PeriodicTask tasks[];
extern int num_tasks;
extern struct node *ready_list;

// Funções do EDF
void initialize_tasks(void);
void check_task_arrivals(int current_time);
void *edf_scheduler(void *arg);

#endif // EDF_H
