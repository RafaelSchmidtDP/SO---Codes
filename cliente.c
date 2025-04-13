#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "banco.h"
#define MAX_BUFFER_SIZE 256

int main() {
    int pipe_fd; // Declara uma variável inteira para armazenar o descritor de arquivo do pipe.
    char *pipe_path = "/tmp/meu_banco_pipe"; // Define o caminho do pipe nomeado que será usado para comunicação.
    char buffer[MAX_BUFFER_SIZE]; // armazenar entrada do usuario

    // Abre o pipe para escrita
    pipe_fd = open(pipe_path, O_WRONLY); // abre o pipe nomeado para escrita WriteOnly
    if (pipe_fd == -1) { // se não deu de abrir open retorna -1
        perror("open pipe");
        exit(1);
    }
  // Envia as requisições
  printf("Digite as requisições no formato:\n");
  printf("INSERT id nome\nDELETE id\nUPDATE id nome\nSELECT id\n");
 
  
  while (fgets(buffer, MAX_BUFFER_SIZE, stdin) != NULL) { // Loop que lê linhas da entrada padrão (stdin) até encontrar o fim do arquivo (EOF) ou um erro.
      buffer[strcspn(buffer, "\n")] = 0; // Retorna pos do \n e remove o newline (entrada: INSERT 1 Rafael -> "INSERT 1 Rafael\n\0")
      write(pipe_fd, buffer, strlen(buffer) + 1);
      printf("...\n");
  }

    return 0;
}