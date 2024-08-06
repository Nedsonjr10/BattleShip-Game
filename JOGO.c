#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define TAMANHO_TABULEIRO 10
#define MAXIMO_NAVIOS 5
#define TAMANHO_HASH 100
typedef struct {
    int x;
    int y;
} Posicao;
typedef struct {
    Posicao inicio;
    Posicao fim;
    int tamanho;
    bool afundado;
} Navio;
typedef struct {
    int id_jogador;
    Navio navios[MAXIMO_NAVIOS];
    int acertos;
    int erros;
    char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
     char tabuleiro_visual[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];  // Mapa visual para que o jogo rode bem
} Jogador;
typedef struct No {
    Jogador jogador;
    struct No *proximo;
} No;
typedef struct NoNavio {
    Navio navio;
    struct NoNavio *anterior;
    struct NoNavio *proximo;
} NoNavio;
typedef struct Acao {
    Posicao pos;
    bool acertou;
    struct Acao *proxima;
} Acao;
typedef struct NoEvento {
    Acao acao;
    struct NoEvento *proximo;
} NoEvento;
typedef struct {
    bool acertos[TAMANHO_HASH];
} TabelaHash;
// Criação do NO 
No* criar_no(Jogador jogador) {
    No *novo_no = (No *)malloc(sizeof(No));
    novo_no->jogador = jogador;
    novo_no->proximo = novo_no;
    return novo_no;
}
Acao* criar_acao(Posicao pos, bool acertou) {
    Acao *nova_acao = (Acao *)malloc(sizeof(Acao));
    nova_acao->pos = pos;
    nova_acao->acertou = acertou;
    nova_acao->proxima = NULL;
    return nova_acao;
}
NoEvento* criar_no_evento(Acao acao) {
    NoEvento *novo_no = (NoEvento *)malloc(sizeof(NoEvento));
    novo_no->acao = acao;
    novo_no->proximo = NULL;
    return novo_no;
}
// Função Hashing
int hash_posicao(Posicao pos) {
    return (pos.x * 10 + pos.y) % TAMANHO_HASH;
}
void registrar_acerto(TabelaHash *tabela, Posicao pos) {
    int indice = hash_posicao(pos);
    tabela->acertos[indice] = true;
}
bool verificar_acerto(TabelaHash *tabela, Posicao pos) {
    int indice = hash_posicao(pos);
    return tabela->acertos[indice];
}
// Tentativa de primeiras lógicas da Lista 
NoNavio* criar_no_navio(Navio navio) {
    NoNavio *novo_no = (NoNavio *)malloc(sizeof(NoNavio));
    novo_no->navio = navio;
    novo_no->anterior = novo_no->proximo = NULL;
    return novo_no;
}
void inserir_navio_frente(NoNavio **cabeca, Navio navio) {
    NoNavio *novo_no = criar_no_navio(navio);
    if (*cabeca == NULL) {
        *cabeca = novo_no;
    } else {
        novo_no->proximo = *cabeca;
        (*cabeca)->anterior = novo_no;
        *cabeca = novo_no;
    }
}

typedef struct Queue {
    NoEvento *front;
    NoEvento *rear;
} Queue;

void enqueue(Queue *q, Acao acao) {
    NoEvento *novo_no = criar_no_evento(acao);
    if (q->rear == NULL) {
        q->front = q->rear = novo_no;
        return;
    }
    q->rear->prox = novo_no;
    q->rear = novo_no;
}

Acao dequeue(Queue *q) {
    if (q->front == NULL) {
        exit(EXIT_FAILURE);
    }
    NoEvento *temp = q->front;
    Acao acao = temp->acao;
    q->front = q->front->prox;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return acao;
}

// Função para imprimir o mapa de jogo atual
void imprimir_tabuleiro(char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO]) {
    printf("  ");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        printf("%d ", i);
    }
    printf("\n");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        printf("%d ", i);
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            printf("%c ", tabuleiro[i][j]);
        }
        printf("\n");
    }
}
int main() {
    No *jogadores = NULL;  
    TabelaHash tabela_hash = {0};  // Tabela hash para registrar tiros
    NoEvento *lista_eventos = NULL;  // Lista simples para registrar eventos
    NoEvento *fila_eventos_frente = NULL, *fila_eventos_tras = NULL; 
    // Inicializar jogadores
    for (int i = 0; i < 2; i++) {
        Jogador novo_jogador;
        novo_jogador.id_jogador = i + 1;
        // Inicializar o tabuleiro e os navios
        for (int x = 0; x < TAMANHO_TABULEIRO; x++) {
            for (int y = 0; y < TAMANHO_TABULEIRO; y++) {
                novo_jogador.tabuleiro[x][y] = '-';
            }
        }
        No *novo_no = criar_no(novo_jogador);
        // Suponha que todos os jogadores já estejam inseridos na lista circular para simplificação
        if (!jogadores) {
            jogadores = novo_no;
            jogadores->proximo = jogadores;
        } else {
            novo_no->proximo = jogadores->proximo;
            jogadores->proximo = novo_no;
        }
    }
    bool jogo_terminado = false;
    No *jogador_atual = jogadores;
    while (!jogo_terminado) {
        printf("Turno do jogador %d\n", jogador_atual->jogador.id_jogador);
        int x, y;
        printf("Insira a coordenada X para atirar: ");
        scanf("%d", &x);
        printf("Insira a coordenada Y para atirar: ");
        scanf("%d", &y);
        Posicao tiro = {x, y};
        if (x < 0 || x >= TAMANHO_TABULEIRO || y < 0 || y >= TAMANHO_TABULEIRO) {
            printf("Coordenadas inválidas. Tente novamente.\n");
            continue;
        }
        if (verificar_acerto(&tabela_hash, tiro)) {
            printf("Posição já foi atacada. Escolha outra.\n");
            continue;
        }
        registrar_acerto(&tabela_hash, tiro);
        bool acertou = rand() % 2;  // Simular um acerto ou erro
        Acao nova_acao = {tiro, acertou};
        NoEvento *novo_evento = criar_no_evento(nova_acao);
        novo_evento->proximo = lista_eventos;
        lista_eventos = novo_evento;
        printf("Resultado do tiro: %s\n", acertou ? "Acertou" : "Errou");
        jogador_atual = jogador_atual->proximo;
        if (jogador_atual == jogadores) {
            printf("----Todos os jogadores jogaram. Nova rodada vai começar.----\n");
        }
        jogo_terminado = (rand() % 10) < 1;  
    }
    printf("Fim do jogo!\n");
    return 0;
}