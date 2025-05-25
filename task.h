typedef struct {
    char *name;
    int tid;
    int priority;
    int burst;
    int waiting_time;
    int remaining;
    int periodo;
    int deadline_rel;   // Deadline relativo (fixo)
    int release_time;   // Tempo em que essa instância foi liberada
    int deadline;       // Deadline absoluto da instância atual
    int next_release;   // Próximo tempo de liberação
    int release_count;
} Task;
