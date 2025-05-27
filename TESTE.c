#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers_rr.h" 
#include "list.h"     
#include "task.h"     
#include "timer.h"     


int main() {

    add("Tarefa1", 3, 9);
    add("Tarefa2", 2, 6);
    add("Tarefa3", 1, 15);
    add("Tarefa4", 4, 11);

    printf("\n--- Starting Scheduling Simulation ---\n");

    // Invoca o escalonador
    schedule();
    
    return 0;
}