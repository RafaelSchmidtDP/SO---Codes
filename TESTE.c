#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers_edf.h" 
#include "list.h"     
#include "task.h"     
#include "timer.h"     


int main() {
/*
    add("Tarefa1", 3, 9);
    add("Tarefa2", 2, 6);
    add("Tarefa3", 1, 15);
    add("Tarefa4", 4, 11);

    printf("\n--- Starting Scheduling Simulation ---\n");

    // Invoca o escalonador
    schedule();
    */
 pthread_t edf_thread;

    timer_set_quantum(1);  // Define quantum de 1 tick
    initialize_tasks();    // Inicializa tarefas periódicas

    // Cria o escalonador EDF em uma thread
    pthread_create(&edf_thread, NULL, edf_scheduler, NULL);

    // Espera o escalonador terminar
    pthread_join(edf_thread, NULL);
    return 0;
}