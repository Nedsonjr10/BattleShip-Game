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
