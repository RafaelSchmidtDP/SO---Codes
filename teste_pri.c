#include <stdio.h>
#include <stdlib.h>

// --- Estruturas ---

struct Processo {
    int id;
    int surto;
    int prioridade;
    int tempoRestante;
    int tempoEspera;
    int tempoExecucao;
    int tempoChegada;
    struct Processo *prox;
};

// --- Funções de Utilitário ---

struct Processo *criarProcesso(int id, int surto, int prioridade, int tempoChegada) {
    struct Processo *novoProcesso = (struct Processo *)malloc(sizeof(struct Processo));
    if (novoProcesso == NULL) {
        printf("Erro Fatal: Falha na alocacao de memoria.\n");
        exit(1);
    }
    novoProcesso->id = id;
    novoProcesso->surto = surto;
    novoProcesso->prioridade = prioridade;
    novoProcesso->tempoRestante = surto;
    novoProcesso->tempoEspera = 0;
    novoProcesso->tempoExecucao = 0;
    novoProcesso->tempoChegada = tempoChegada;
    novoProcesso->prox = NULL;
    return novoProcesso;
}

void adicionarProcessoNaFila(struct Processo **fila, struct Processo *processo) {
    if (*fila == NULL) {
        *fila = processo;
    } else {
        struct Processo *temp = *fila;
        while (temp->prox != NULL) {
            temp = temp->prox;
        }
        temp->prox = processo;
    }
    processo->prox = NULL;
}

struct Processo *removerPrimeiroDaFila(struct Processo **fila) {
    if (*fila == NULL) {
        return NULL;
    }
    struct Processo *primeiro = *fila;
    *fila = (*fila)->prox;
    primeiro->prox = NULL;
    return primeiro;
}

void liberarLista(struct Processo *lista) {
    struct Processo *temp;
    while (lista != NULL) {
        temp = lista;
        lista = lista->prox;
        free(temp);
    }
}

void imprimirFilas(struct Processo *filasDePrioridade[], int numPrioridades, int tempoAtual) {
    printf("\n=== Tempo Atual: %d ===\n", tempoAtual);
    for (int i = 0; i < numPrioridades; i++) {
        printf("Fila Prioridade %d: ", i);
        struct Processo *temp = filasDePrioridade[i];
        if (temp == NULL) {
            printf("Vazia");
        }
        while (temp != NULL) {
            printf("[P%d TR:%d TE:%d] -> ", temp->id, temp->tempoRestante, temp->tempoEspera);
            temp = temp->prox;
        }
        printf("NULL\n");
    }
}

// --- Algoritmo Round Robin Multinível com Prioridade ---

void roundRobinComPrioridade(struct Processo *processosIniciais, int quantum, int numPrioridades) {
    printf("\n--- Escalonamento Round Robin Multinivel com Prioridade ---\n");
    printf("Quantum: %d, Niveis de Prioridade: %d (0 = mais alta)\n\n", quantum, numPrioridades);

    struct Processo *filasDePrioridade[numPrioridades];
    for (int i = 0; i < numPrioridades; i++) {
        filasDePrioridade[i] = NULL;
    }

    struct Processo *temp = processosIniciais;
    while (temp != NULL) {
        struct Processo *copia = criarProcesso(temp->id, temp->surto, temp->prioridade, temp->tempoChegada);
        if (copia->prioridade >= 0 && copia->prioridade < numPrioridades) {
            adicionarProcessoNaFila(&filasDePrioridade[copia->prioridade], copia);
        } else {
            printf("Aviso: Processo %d com prioridade invalida %d. Ignorado.\n", copia->id, copia->prioridade);
            free(copia);
        }
        temp = temp->prox;
    }

    int tempoAtual = 0;
    int processosConcluidos = 0;

    struct Processo *contagemTemp = processosIniciais;
    int totalProcessos = 0;
    while(contagemTemp != NULL) {
        totalProcessos++;
        contagemTemp = contagemTemp->prox;
    }

    while (processosConcluidos < totalProcessos) {
        int processoEncontradoNaRodada = 0;

        imprimirFilas(filasDePrioridade, numPrioridades, tempoAtual);

        for (int i = 0; i < numPrioridades; i++) {
            if (filasDePrioridade[i] != NULL) {
                struct Processo *processoAtual = removerPrimeiroDaFila(&filasDePrioridade[i]);

                int tempoParaExecutar = (processoAtual->tempoRestante > quantum) ? quantum : processoAtual->tempoRestante;

                printf("\nExecutando Processo %d (Prioridade %d): ", processoAtual->id, processoAtual->prioridade);
                printf("Tempo Restante Antes = %d, Executando = %d\n", processoAtual->tempoRestante, tempoParaExecutar);

                for (int j = 0; j < numPrioridades; j++) {
                    struct Processo *filaTemp = filasDePrioridade[j];
                    while (filaTemp != NULL) {
                        if (filaTemp->id != processoAtual->id) {
                            if (filaTemp->tempoChegada <= tempoAtual) {
                                filaTemp->tempoEspera += tempoParaExecutar;
                            }
                        }
                        filaTemp = filaTemp->prox;
                    }
                }

                processoAtual->tempoRestante -= tempoParaExecutar;
                processoAtual->tempoExecucao += tempoParaExecutar;
                tempoAtual += tempoParaExecutar;

                processoEncontradoNaRodada = 1;

                if (processoAtual->tempoRestante == 0) {
                    printf("Processo %d CONCLUIDO. Tempo de Surto: %d, Espera: %d, Turnaround: %d\n",
                           processoAtual->id, processoAtual->surto, processoAtual->tempoEspera,
                           processoAtual->tempoExecucao + processoAtual->tempoEspera);
                    free(processoAtual);
                    processosConcluidos++;
                } else {
                    adicionarProcessoNaFila(&filasDePrioridade[i], processoAtual);
                    printf("Processo %d NAO CONCLUIDO. Tempo Restante Agora = %d\n", processoAtual->id, processoAtual->tempoRestante);
                }
                break;
            }
        }

        if (!processoEncontradoNaRodada && processosConcluidos < totalProcessos) {
            tempoAtual++;
        }

        if (tempoAtual > 10000 && processosConcluidos < totalProcessos) {
            printf("Erro: Loop infinito detectado. Saindo.\n");
            break;
        }
    }

    for (int i = 0; i < numPrioridades; i++) {
        liberarLista(filasDePrioridade[i]);
    }
}

// --- Função Principal ---

int main() {
    struct Processo *listaDeChegada = NULL;

    adicionarProcessoNaFila(&listaDeChegada, criarProcesso(1, 10, 2, 0));
    adicionarProcessoNaFila(&listaDeChegada, criarProcesso(2, 5, 0, 1));
    adicionarProcessoNaFila(&listaDeChegada, criarProcesso(3, 8, 1, 2));
    adicionarProcessoNaFila(&listaDeChegada, criarProcesso(4, 3, 2, 3));
    adicionarProcessoNaFila(&listaDeChegada, criarProcesso(5, 7, 0, 4));

    int quantum = 3;
    int numPrioridades = 3;

    roundRobinComPrioridade(listaDeChegada, quantum, numPrioridades);

    liberarLista(listaDeChegada);

    return 0;
}
