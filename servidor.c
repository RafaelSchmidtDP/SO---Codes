#include <stdio.h>    // funções E/S
#include <stdlib.h>   //Funções utilitárias(malloc,free,exit)
#include <unistd.h>   // Funções POSIX
#include <pthread.h>  // Threads
#include <string.h>   // Manipulação d string
#include <fcntl.h>    // Controle de arquivos (open, O_RDONLY, O_WRONLY, etc.)
#include <sys/stat.h> // Operações com arquivos e permissões (mkfifo, stat, chmod, etc.)
#include <errno.h>    // Manipulação de erros do sistema
#include "banco.h" 

#define NUM_TASKS 5 // Tamanho máximo da fila de tarefas

// Estrutura de uma tarefa
typedef struct {
    char data[MAX_BUFFER_SIZE]; // Armazena os dados da tarefa
    int tamanho;                 // Armazena o tamanho real dos dados -> 100 caracteres lidos = tamanho 100
} Task;

Task queue[NUM_TASKS]; // Fila de tarefas para as threads
int task_count = 0;     // Número de tarefas atuais na fila

// Variáveis de sincronização para controle da fila
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;

// Banco de dados (vetor de registros) -> TABELA DE DADOS
Registro banco[MAX_REGISTROS];
int num_registros = 0;
pthread_mutex_t banco_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para acesso ao banco

// Funções de manipulação do banco de dados
int inicializarBanco();
int salvarBanco();
int inserirRegistro(Registro novo_registro);
int removerRegistro(int id);
int encontrarRegistro(int id);

// Thread principal que ficará escutando e processando tarefas
void* startThread(void *args);

// Processamento de uma única requisição
void processar_requisicao(Task task);

// Adiciona uma tarefa à fila de execução
void add_task(const char *buffer, int tamanho);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Lê o banco de dados a partir de um arquivo
int inicializarBanco() {
    FILE *arquivo = fopen(DATABASE_FILE, "r"); // abre arquivo para leitura
    if (arquivo == NULL) {
        return 0; // Arquivo não existe ainda
    }

    int i = 0;
    // Formato: inteiro e string com até 49 caracteres -> armazena no banco
    // CARREGA DADOS DO ARQUIVO PARA MEMORIA (BANCO)
    while (fscanf(arquivo, "%d %49s", &banco[i].id, banco[i].nome) == 2 && i < MAX_REGISTROS) {
        i++;
    }

    fclose(arquivo);

    num_registros = i;
    return num_registros;
}

// Salva o conteúdo do banco no arquivo
int salvarBanco() {
    FILE *arquivo = fopen(DATABASE_FILE, "w"); // SE arquivo nao existir, ele sera criado.
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo do banco de dados para escrita");
        return -1;
    }
    // percorre o banco printando o id e nome\n
    // SALVA O QUE TA NA MEMORIA(BANCO) NO ARQUIVO
    for (int i = 0; i < num_registros; i++) {
        fprintf(arquivo, "%d %s\n", banco[i].id, banco[i].nome);
    }

    fclose(arquivo);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Insere novo registro no banco
int inserirRegistro(Registro novo_registro) {
    pthread_mutex_lock(&banco_mutex);

    if (num_registros < MAX_REGISTROS) {
        banco[num_registros] = novo_registro;
        num_registros++;
        salvarBanco();
        pthread_mutex_unlock(&banco_mutex);
        return 0;
    } else {
        pthread_mutex_unlock(&banco_mutex);
        return -1; // Banco cheio
    }
}

// Atualiza registro existente
int update(Registro update_registro) {
    pthread_mutex_lock(&banco_mutex);
    int indice = encontrarRegistro(update_registro.id);

    if (indice != -1) {
        banco[indice] = update_registro;
        salvarBanco();
        pthread_mutex_unlock(&banco_mutex);
        return 0;
    } else {
        pthread_mutex_unlock(&banco_mutex);
        return -1; // Registro não encontrado
    }
}

// Remove registro com determinado ID
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
        return 0;
    } else {
        pthread_mutex_unlock(&banco_mutex);
        return -1;
    }
}

// Busca o índice de um registro pelo ID
int encontrarRegistro(int id) {
    for (int i = 0; i < num_registros; i++) {
        if (banco[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Verifica se já existe um registro com o mesmo ID (para evitar duplicidade)
int verificar_consistencia_id(Registro registro_consistencia) {
    pthread_mutex_lock(&banco_mutex);
    for (int i = 0; i < num_registros; i++) {
        if (registro_consistencia.id == banco[i].id) {
            pthread_mutex_unlock(&banco_mutex);
            return -1; // ID já existe
        }
    }
    pthread_mutex_unlock(&banco_mutex);
    return 1; // ID válido
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Função executada por cada thread: processa tarefas da fila
void* startThread(void* args) {
    while (1) {
        Task task;

        // Trava o mutex para garantir acesso exclusivo à fila de tarefas
        pthread_mutex_lock(&mutex);

        // Enquanto não houver tarefas na fila, a thread aguarda (bloqueia) até que uma nova tarefa seja adicionada
        while (task_count == 0) {
            pthread_cond_wait(&condvar, &mutex); // Aguarda nova tarefa, desbloqueando e aguardando sinalização
        }

        // Remove a primeira tarefa da fila
        task = queue[0];
        
        // Move todas as tarefas para frente na fila (para "remover" a tarefa da frente)
        for (int i = 0; i < task_count - 1; i++) {
            queue[i] = queue[i + 1]; // Desloca as tarefas
        }

        // Decrementa o número de tarefas na fila
        task_count--;

        // Desbloqueia o mutex para permitir o acesso concorrente à fila por outras threads
        pthread_mutex_unlock(&mutex);

        // Processa a requisição da tarefa (essa função será responsável pela execução da tarefa)
        processar_requisicao(task); // Executa a tarefa
    }
    return NULL; // Retorna NULL ao final da execução (nunca será alcançado, pois o loop é infinito)
}


// Interpreta e executa uma requisição recebida
void processar_requisicao(Task task) {
    char requisicao[MAX_BUFFER_SIZE];
    memcpy(requisicao, task.data, task.tamanho);

    char tipo[10];
    int id;
    char nome[MAX_NAME_SIZE];

    // Comando com ID e Nome (INSERT e UPDATE)
    if (sscanf(requisicao, "%s %d %49s", tipo, &id, nome) == 3) {
        if (strcmp(tipo, "INSERT") == 0) {

            Registro novo_registro = {id, ""};

            if (verificar_consistencia_id(novo_registro) == 1) {
                strcpy(novo_registro.nome, nome);

                if(inserirRegistro(novo_registro) == 0){
                    printf("Registro inserido: ID = %d, Nome = %s\n", id, nome);
                }
                else{
                    printf("Banco cheio");
                }
            } 

            else {
                printf("Registro não pode ser inserido -> MOTIVO: ID repetido\n");
            }

        } else if (strcmp(tipo, "UPDATE") == 0) {

            Registro registro_atualizado = {id, ""};
            strcpy(registro_atualizado.nome, nome);

            if (update(registro_atualizado) == 0) {
                printf("Registro atualizado: ID = %d, Nome = %s\n", id, nome);
            } 
            else {
                printf("Registro com ID %d não encontrado para atualização.\n", id);
            }

        } else {
            printf("Comando inválido: %s\n", tipo);
        }

    // Comando com apenas ID (ex: DELETE, SELECT)
    } else if (sscanf(requisicao, "%s %d", tipo, &id) == 2) {
        if (strcmp(tipo, "DELETE") == 0) {

            int indice = encontrarRegistro(id);

            if (indice != -1) {
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

// Adiciona uma nova tarefa à fila (executado pelo produtor)
void add_task(const char *buffer, int tamanho) {
    pthread_mutex_lock(&mutex);

    if (task_count < NUM_TASKS) {
        // Copia os dados do buffer para o campo 'data' da tarefa na fila
        // Isso armazena os dados da tarefa para que possam ser processados por uma thread
        memcpy(queue[task_count].data, buffer, tamanho);

        // Define o tamanho da tarefa
        queue[task_count].tamanho = tamanho;

        // Incrementa o número de tarefas na fila
        task_count++;

        // Notifica uma das threads de que há uma nova tarefa para ser processada
        pthread_cond_signal(&condvar); // Notifica uma thread que há tarefa
    } else {
        // Caso a fila esteja cheia, imprime uma mensagem de erro
        fprintf(stderr, "Fila de tarefas cheia! Não foi possível adicionar tarefa.\n");
    }

    pthread_mutex_unlock(&mutex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    pthread_t th[THREAD_NUM];                  // Array para armazenar os identificadores das threads
    int pipe_fd;                               // Descritor de arquivo para o pipe nomeado
    char *pipe_path = "/tmp/meu_banco_pipe";  // Caminho para o pipe no sistema de arquivos
    char buffer[MAX_BUFFER_SIZE];            // Buffer para armazenar os dados lidos do pipe
    int i;

    // Cria o pipe nomeado com permissões 0666 (leitura e escrita para todos os usuários)
    if (mkfifo(pipe_path, 0666) == -1) {
        if (errno != EEXIST) { //Se ocorreu erro e o erro **não** foi "arquivo já existe" (EEXIST)
            perror("mkfifo");
            exit(1);
        }
    }

    // Inicializa o banco de dados
    inicializarBanco();

    // Cria as threads do pool
    for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_create(&th[i], NULL, &startThread, NULL) != 0) { //
            perror("Failed to create the thread");
            exit(1);
        }
    }

    // Abre o pipe para leitura
    pipe_fd = open(pipe_path, O_RDONLY);
    if (pipe_fd == -1) {
        perror("open pipe");
        exit(1);
    }

    // Loop principal de leitura do pipe (consumidor)
    ssize_t bytes_read;
    while ((bytes_read = read(pipe_fd, buffer, MAX_BUFFER_SIZE)) > 0) {
        add_task(buffer, bytes_read);
    }

    if (bytes_read == 0) {
        printf("Cliente encerrou a comunicação.\n");
        exit(0);
    } else if (bytes_read == -1) {
        perror("Erro ao ler do pipe");
        exit(1);
    }

    close(pipe_fd);
    unlink(pipe_path); // Remove o pipe do sistema

    // Aguarda as threads finalizarem (nunca vai acontecer neste caso)
    for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }

    return 0;
}
