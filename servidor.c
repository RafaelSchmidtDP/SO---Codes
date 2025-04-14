#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "banco.h" 

#define NUM_TASKS 5

// Fila de tarefas
typedef struct {
    char data[MAX_BUFFER_SIZE];
    int tamanho;
} Task;

Task queue[NUM_TASKS];
int task_count = 0;
int task_index = 0;

// Variáveis de sincronização
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;

Registro banco[MAX_REGISTROS];
int num_registros = 0;
pthread_mutex_t banco_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funções de manipulação do banco de dados (agora em servidor.c)
int inicializarBanco();
int salvarBanco();
int inserirRegistro(Registro novo_registro);
int removerRegistro(int id);
int encontrarRegistro(int id);

// Função para iniciar a thread e aguardar tarefas
void* startThread(void *args); // Sem ponteiro no argumento

// Função para processar uma requisição
void processar_requisicao(Task task);

// Adiciona uma tarefa à fila
void add_task(const char *buffer, int tamanho);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int inicializarBanco() {
    FILE *arquivo = fopen(DATABASE_FILE, "r");
    if (arquivo == NULL) {
        return 0;
    }

    int i = 0;
    // le um formato especifico -> inteiro e uma string de 49 caracteres -> retorna 2 se leu corretamente
    while (fscanf(arquivo, "%d %49s", &banco[i].id, banco[i].nome) == 2 && i < MAX_REGISTROS) {
        i++;
    }

    fclose(arquivo);

    num_registros = i; // nao usado
    return num_registros;// nao usado
}

int salvarBanco() {
    FILE *arquivo = fopen(DATABASE_FILE, "w");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo do banco de dados para escrita");
        return -1;
    }

    for (int i = 0; i < num_registros; i++) {
        fprintf(arquivo, "%d %s\n", banco[i].id, banco[i].nome);
    }

    fclose(arquivo);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int inserirRegistro(Registro novo_registro) {
    pthread_mutex_lock(&banco_mutex);
    if (num_registros < MAX_REGISTROS) {
        banco[num_registros] = novo_registro;
        num_registros++;
        salvarBanco();
        pthread_mutex_unlock(&banco_mutex);
        return 0; // Sucesso
    } else {
        pthread_mutex_unlock(&banco_mutex);
        return -1; // Banco de dados cheio
    }
}

int update(Registro update_registro) {
    pthread_mutex_lock(&banco_mutex);
    int indice = encontrarRegistro(update_registro.id);
    if (indice != -1) {
        banco[indice] = update_registro;
        salvarBanco();
        pthread_mutex_unlock(&banco_mutex);
        return 0; // Sucesso
    } else {
        pthread_mutex_unlock(&banco_mutex);
        return -1; // Registro não encontrado para atualização
    }
}

int removerRegistro(int id) {
    pthread_mutex_lock(&banco_mutex);
    int indice = encontrarRegistro(id);
    if (indice != -1) {
        for (int i = indice; i < num_registros - 1; i++) {
            banco[i] = banco[i + 1];
        }
        num_registros--;
        salvarBanco();
        pthread_mutex_unlock(&banco_mutex);
        return 0; // Sucesso
    } else {
        pthread_mutex_unlock(&banco_mutex);
        return -1; // Registro não encontrado
    }
}

int encontrarRegistro(int id) {
    for (int i = 0; i < num_registros; i++) {
        if (banco[i].id == id) {
            return i; // Retorna o índice do registro
        }
    }
    return -1; // Registro não encontrado
}

int verificar_consistencia_id(Registro registro_consistencia) {
    pthread_mutex_lock(&banco_mutex);
    for (int i = 0; i < num_registros; i++) {
        if (registro_consistencia.id == banco[i].id) {
            pthread_mutex_unlock(&banco_mutex);
            return -1; // Há um registro com o mesmo id
        }
    }
    pthread_mutex_unlock(&banco_mutex);
    return 1; // Tudo certo
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* startThread(void* args) {
    while (1) {
        Task task;

        // Bloqueia o mutex para acessar a fila de tarefas
        pthread_mutex_lock(&mutex);

        // Espera até que uma tarefa esteja disponível
        while (task_count == 0) {
            pthread_cond_wait(&condvar, &mutex);
            //printf("Thread %ld: Aguardando por tarefas...\n", pthread_self());
        }

        // Retira a próxima tarefa da fila (movendo as tarefas para frente)
        task = queue[0];  // Pega a primeira tarefa
        for (int i = 0; i < task_count - 1; i++) {
            queue[i] = queue[i + 1];  // Move as tarefas para frente
        }
        task_count--;  // Decrementa o número de tarefas na fila

        pthread_mutex_unlock(&mutex);  // Desbloqueia o mutex

        // Processa a tarefa
        processar_requisicao(task);
    }
    return NULL;
}

void processar_requisicao(Task task) {
    char requisicao[MAX_BUFFER_SIZE];
    memcpy(requisicao, task.data, task.tamanho);

    // Processa a requisição
    char tipo[10];
    int id;
    char nome[MAX_NAME_SIZE];
    if (sscanf(requisicao, "%s %d %49s", tipo, &id, nome) == 3) {
        if (strcmp(tipo, "INSERT") == 0) {
            Registro novo_registro = {id, ""};
            if (verificar_consistencia_id(novo_registro) == 1) {
                strcpy(novo_registro.nome, nome);
                inserirRegistro(novo_registro);
                printf("Registro inserido: ID = %d, Nome = %s\n", id, nome);
            } else {
                printf("Registro não pode ser inserido -> MOTIVO: ID repetido\n");
            }
        } else if (strcmp(tipo, "UPDATE") == 0) {
            Registro registro_atualizado = {id, ""};
            strcpy(registro_atualizado.nome, nome);
            if (update(registro_atualizado) == 0) {
                printf("Registro atualizado: ID = %d, Nome = %s\n", id, nome);
            } else {
                printf("Registro com ID %d não encontrado para atualização.\n", id);
            }
        } else {
            printf("Comando inválido: %s\n", tipo);
        }
    } else if (sscanf(requisicao, "%s %d", tipo, &id) == 2) {
        if (strcmp(tipo, "DELETE") == 0) {
            int indice = encontrarRegistro(id);  // Busca o registro pelo id
            if (indice != -1) {  // Se o índice for diferente de -1, o registro existe
                removerRegistro(id);
                printf("Registro removido: ID = %d\n", id);
            } else {
                printf("Registro a ser deletado não existe!\n");
            }
        } else if (strcmp(tipo, "SELECT") == 0) {
            int id_encontrado = encontrarRegistro(id);
            if (id_encontrado != -1) {
                printf("Registro encontrado: ID = %d, Nome = %s\n", banco[id_encontrado].id, banco[id_encontrado].nome);
            } else {
                printf("Registro com ID %d não encontrado.\n", id);
            }
        } else {
            printf("Comando inválido: %s\n", tipo);
        }
    } else {
        printf("Formato de requisição inválido: %s\n", requisicao);
    }
}

// Adiciona uma tarefa à fila
void add_task(const char *buffer, int tamanho) {
    pthread_mutex_lock(&mutex);

    if (task_count < NUM_TASKS) {
        memcpy(queue[task_count].data, buffer, tamanho);
        queue[task_count].tamanho = tamanho;
        task_count++;
        pthread_cond_signal(&condvar); // Notifica as threads que há uma nova tarefa
    } else {
        fprintf(stderr, "Fila de tarefas cheia! Não foi possível adicionar tarefa.\n");
    }

    pthread_mutex_unlock(&mutex);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    pthread_t th[THREAD_NUM];
    int pipe_fd;
    char *pipe_path = "/tmp/meu_banco_pipe";
    char buffer[MAX_BUFFER_SIZE];
    int i;

    // Cria o pipe
    if (mkfifo(pipe_path, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    // Inicializa o banco de dados
    inicializarBanco();

    // Cria as threads no pool
    for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_create(&th[i], NULL, &startThread, NULL) != 0) {
            perror("Failed to create the thread");
            exit(EXIT_FAILURE);
        }
    }

    // Abre o pipe para leitura
    pipe_fd = open(pipe_path, O_RDONLY);
    if (pipe_fd == -1) {
        perror("open pipe");
        exit(EXIT_FAILURE);
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Lê as requisições do pipe e as enfileira como tarefas
    ssize_t bytes_read;
    while ((bytes_read = read(pipe_fd, buffer, MAX_BUFFER_SIZE)) > 0) {
        add_task(buffer, bytes_read);
    }

    close(pipe_fd);
    unlink(pipe_path); // Remove o pipe

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Aguarda as threads terminarem
    for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }

    return 0;
}