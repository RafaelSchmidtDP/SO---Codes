#include <stdio.h>
#include <stdlib.h>

#define MAX_TASKS 100
#define MAX_TIME 1000  // Ajuste o tempo de simulação que desejar

typedef struct {
    int id;
    int execution;
    int period;
    int deadline;
    int arrival;
} Task;

typedef struct {
    Task task;
    int release_time;
} Instance;

int main() {
    Task tasks[MAX_TASKS];
    int n;

    printf("Enter the number of tasks: ");
    scanf("%d", &n);

    // Leitura das tarefas
    for (int i = 0; i < n; i++) {
        tasks[i].id = i + 1;
        printf("Task %d execution time: ", i + 1);
        scanf("%d", &tasks[i].execution);
        printf("Task %d period: ", i + 1);
        scanf("%d", &tasks[i].period);
        printf("Task %d deadline: ", i + 1);
        scanf("%d", &tasks[i].deadline);
        printf("Task %d arrival time: ", i + 1);
        scanf("%d", &tasks[i].arrival);
    }

    printf("\n--- Task Set ---\n");
    for (int i = 0; i < n; i++) {
        printf("Task %d -> Exec: %d, Period: %d, Deadline: %d, Arrival: %d\n",
            tasks[i].id, tasks[i].execution, tasks[i].period, tasks[i].deadline, tasks[i].arrival);
    }

    // Cálculo de utilização
    double utilization = 0;
    for (int i = 0; i < n; i++) {
        utilization += (double)tasks[i].execution / tasks[i].period;
    }
    printf("\nTotal Utilization: %.2lf\n", utilization);
    if (utilization > 1.0) {
        printf("Warning: Task set is not feasible (utilization > 1)\n");
    }

    // Inicializar vetor de tempo restante para execução das tarefas
    int time_left[MAX_TASKS] = {0};
    for (int i = 0; i < n; i++) {
        if (tasks[i].arrival == 0) {
            time_left[i] = tasks[i].execution;
        }
    }

    // Timeline de execução, 0 = CPU idle
    int timeline[MAX_TIME];
    int timeline_len = 0;

    printf("\n--- Starting Scheduling Simulation ---\n");

    for (int time = 0; time < MAX_TIME; time++) {
        // Atualizar execuções das tarefas liberadas nesse tempo (arrival + k*period)
        for (int i = 0; i < n; i++) {
            if (time >= tasks[i].arrival && (time - tasks[i].arrival) % tasks[i].period == 0) {
                time_left[i] = tasks[i].execution;
            }
        }

        // Escalonar tarefa com menor deadline (EDF) que esteja pronta para executar
        int task_to_run = 0;  // 0 significa CPU idle
        int earliest_deadline = 1e9;

        for (int i = 0; i < n; i++) {
            if (time_left[i] > 0) {
                // Calcular deadline da instância atual
                int current_deadline = tasks[i].arrival + ((time - tasks[i].arrival) / tasks[i].period + 1) * tasks[i].period;
                if (current_deadline < earliest_deadline) {
                    earliest_deadline = current_deadline;
                    task_to_run = tasks[i].id;
                }
            }
        }

        if (task_to_run != 0) {
            int idx = task_to_run - 1;
            time_left[idx]--;
        }

        timeline[timeline_len++] = task_to_run;
    }

    // Imprimir timeline agrupando execuções consecutivas iguais
    printf("\n--- Timeline (Format: StartTime EndTime [TaskID]) ---\n");
    int start = 0;
    for (int i = 1; i <= timeline_len; i++) {
        if (i == timeline_len || timeline[i] != timeline[i-1]) {
            printf("%4d %4d [Task %d]\n", start, i, timeline[i-1]);
            start = i;
        }
    }

    printf("\nSimulation finished after %d time units.\n", MAX_TIME);

    return 0;
}
