#include <stdio.h> // Para printf

#include "CPU.h"   // Inclui o próprio cabeçalho
#include "task.h"  // Inclui a estrutura Task
#include "timer.h" // Inclui o tempo global

// run this task for the specified time slice
void run(Task *task, int slice) {
    printf("Running task [%s] (TID: %d, Priority: %d, Burst: %d) for %d units.\n",
           task->name, task->tid, task->priority, task->burst, slice);
    // Atualiza o tempo global do sistema
}