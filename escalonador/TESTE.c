#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers_aging.h" 
#include "list.h"     
#include "task.h"     
#include "timer.h"     


int main() {

    add("Tarefa1", 2, 24);
    add("Tarefa2", 4, 3);
    add("Tarefa3", 6, 3);

    printf("\n--- Starting Scheduling Simulation ---\n");

    // Invoca o escalonador
    schedule();
/*
    add("Tarefa1", 1, 50, 5); 
    add("Tarefa2", 1, 8, 10);
    add("Tarefa3", 1, 5, 5);   

     schedule();
  */   
    return 0;
}