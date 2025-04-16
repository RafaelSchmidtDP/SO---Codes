#include <stdio.h>  // funções E/S
#include <stdlib.h>  //Funções utilitárias(malloc,free,exit)
#include <unistd.h> // Funções POSIX
#include <string.h> // Manipulação d string
#include <fcntl.h> // Controle de arquivos (open, O_RDONLY, O_WRONLY, etc.)
#include "banco.h"
//#define MAX_BUFFER_SIZE 256

int main() {
    int pipe_fd;                             // Descritor de arquivo para o pipe nomeado
    char *pipe_path = "/tmp/meu_banco_pipe"; // Caminho para o pipe no sistema de arquivos
    char buffer[MAX_BUFFER_SIZE];            // Buffer para armazenar os dados lidos do pipe
    char comando_sair[] = "SAIR";            // Define o comando para encerrar
    // Abre o pipe para escrita
    pipe_fd = open(pipe_path, O_WRONLY);
    if (pipe_fd == -1) {
        perror("open pipe");
        exit(1);
    }

    // Envia as requisições
    printf("Digite as requisições no formato:\n");
    printf("INSERT id nome\nDELETE id\nUPDATE id nome\nSELECT id\n");
    printf("Digite '%s' para encerrar a comunicação.\n", comando_sair);
    
    while (fgets(buffer, MAX_BUFFER_SIZE, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove o newline

        // Verifica se o comando de sair foi digitado
        if (strcmp(buffer, comando_sair) == 0) {
            printf("Encerrando a comunicação...\n");
            break; // Sai do loop while
        }

        write(pipe_fd, buffer, strlen(buffer) + 1); // strlen nao conta com o \0, por isso +1
        printf("...\n");
    }

/*
for (int i = 0; i < 10; i++) {
    snprintf(buffer, sizeof(buffer), "INSERT %d Nome%d", i + 1, i + 1);
    if (write(pipe_fd, buffer, strlen(buffer) + 1) == -1) {
        perror("write to pipe");
    } else {
        printf("Enviado: %s\n", buffer);
    }
    usleep(100000); // Pequeno delay (100ms) 
}
*/
    // Fecha o descritor de arquivo do pipe
    if (close(pipe_fd) == -1) {
        perror("close pipe");
        exit(1);
    }

    return 0;
}