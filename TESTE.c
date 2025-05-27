#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers_edf.h" 
#include "list.h"     
#include "task.h"     
#include "timer.h"     


int main() {
/*
    add("Tarefa1", 2, 24);
    add("Tarefa2", 4, 3);
    add("Tarefa3", 6, 3);

    printf("\n--- Starting Scheduling Simulation ---\n");

    // Invoca o escalonador
    schedule();
    */
    add_edf("Tarefa1", 1, 15, 10);  // burst 15, deadline 10
    add_edf("Tarefa2", 1, 5, 20);   // burst 5, deadline 20
    add_edf("Tarefa3", 1, 8, 25);   // burst 8, deadline 25

     schedule_edf();
     
    return 0;
}