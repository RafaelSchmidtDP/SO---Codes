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
    timer_set_quantum(1); // Cada tick = 1 unidade de tempo
    timer_start();

    edf_init();

    // Adiciona tarefas dinamicamente
    add("TarefaA", 1, 4, 5);
    add("TarefaB", 1, 3, 6);
    add("TarefaB", 1, 2, 7);


    edf_run();

    edf_cleanup();

    timer_stop();


    edf_run();

    edf_cleanup();
    timer_stop();
    return 0;
}