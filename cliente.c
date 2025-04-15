#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "banco.h"
#define MAX_BUFFER_SIZE 256

int main() {
    int pipe_fd;
    char *pipe_path = "/tmp/meu_banco_pipe";
    char buffer[MAX_BUFFER_SIZE];
    char comando_sair[] = "SAIR"; // Define o comando para encerrar

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

        write(pipe_fd, buffer, strlen(buffer) + 1);
        printf("...\n");
    }

    // Fecha o descritor de arquivo do pipe
    if (close(pipe_fd) == -1) {
        perror("close pipe");
        exit(1);
    }

    return 0;
}