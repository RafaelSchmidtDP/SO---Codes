#ifndef BANCO_H
#define BANCO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_NAME_SIZE 50
#define MAX_BUFFER_SIZE 256
#define DATABASE_FILE "banco.txt" // Nome do arquivo do banco de dados
#define MAX_REGISTROS 100
#define THREAD_NUM 5
// Estrutura para representar um registro no banco de dados
typedef struct {
    int id;
    char nome[MAX_NAME_SIZE];
} Registro;

#endif // BANCO_H