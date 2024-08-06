#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TAMANHO_TABULEIRO 10
#define MAX_NAVIOS 5
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
    Navio navios[MAX_NAVIOS];
    int acertos;
    int erros;
    char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
    char tabuleiro_visivel[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];  // Mapa visual para o jogador
} Jogador;

typedef struct Node {
    Jogador jogador;
    struct Node *proximo;
} Node;

typedef struct NavioNode {
    Navio navio;
    struct NavioNode *anterior;
    struct NavioNode *proximo;
} NavioNode;

typedef struct Acao {
    Posicao posicao;
    bool acerto;
    struct Acao *proximo;
} Acao;

typedef struct EventoNode {
    Acao acao;
    struct EventoNode *proximo;
} EventoNode;

typedef struct {
    bool acertos[TAMANHO_HASH];
} TabelaHash;

Node* criar_node(Jogador jogador) {
    Node *novo_node = (Node *)malloc(sizeof(Node));
    novo_node->jogador = jogador;
    novo_node->proximo = novo_node;
    return novo_node;
}

Acao* criar_acao(Posicao posicao, bool acerto) {
    Acao *nova_acao = (Acao *)malloc(sizeof(Acao));
    nova_acao->posicao = posicao;
    nova_acao->acerto = acerto;
    nova_acao->proximo = NULL;
    return nova_acao;
}

EventoNode* criar_evento_node(Acao acao) {
    EventoNode *novo_node = (EventoNode *)malloc(sizeof(EventoNode));
    novo_node->acao = acao;
    novo_node->proximo = NULL;
    return novo_node;
}

int hash_posicao(Posicao posicao) {
    return (posicao.x * 10 + posicao.y) % TAMANHO_HASH;
}

void registrar_acerto(TabelaHash *tabela, Posicao posicao) {
    int indice = hash_posicao(posicao);
    tabela->acertos[indice] = true;
}

bool verificar_acerto(TabelaHash *tabela, Posicao posicao) {
    int indice = hash_posicao(posicao);
    return tabela->acertos[indice];
}

NavioNode* criar_navio_node(Navio navio) {
    NavioNode *novo_node = (NavioNode *)malloc(sizeof(NavioNode));
    novo_node->navio = navio;
    novo_node->anterior = novo_node->proximo = NULL;
    return novo_node;
}

void inserir_navio_frente(NavioNode **cabeca, Navio navio) {
    NavioNode *novo_node = criar_navio_node(navio);
    if (*cabeca == NULL) {
        *cabeca = novo_node;
    } else {
        novo_node->proximo = *cabeca;
        (*cabeca)->anterior = novo_node;
        *cabeca = novo_node;
    }
}

typedef struct Queue {
    EventoNode *frente;
    EventoNode *fundo;
} Queue;

void enqueue(Queue *q, Acao acao) {
    EventoNode *novo_node = criar_evento_node(acao);
    if (q->fundo == NULL) {
        q->frente = q->fundo = novo_node;
        return;
    }
    q->fundo->proximo = novo_node;
    q->fundo = novo_node;
}

Acao dequeue(Queue *q) {
    if (q->frente == NULL) {
        exit(EXIT_FAILURE);
    }
    EventoNode *temp = q->frente;
    Acao acao = temp->acao;
    q->frente = q->frente->proximo;
    if (q->frente == NULL) {
        q->fundo = NULL;
    }
    free(temp);
    return acao;
}

void print_queue(Queue *q) { /*ADICIONEI AGORA PARA TESTAR O HISTORICO USADO A FILA */
    EventoNode *atual = q->frente;
    printf("Histórico de ações do jogo:\n");
    while (atual != NULL) {
        Acao acao = atual->acao;
        printf("Tiro em (%d, %d) foi um %s.\n",
               acao.posicao.x, acao.posicao.y,
               acao.acerto ? "Hit" : "Miss");
        atual = atual->proximo;
    }
}

typedef struct stack {
    Acao *topo;
} stack;

void push(stack *p, Acao acao) {
    Acao *nova_acao = criar_acao(acao.posicao, acao.acerto);
    nova_acao->proximo = p->topo;
    p->topo = nova_acao;
}

Acao pop(stack *p) {
    if (p->topo == NULL) {
        exit(EXIT_FAILURE);
    }
    Acao *temp = p->topo;
    Acao acao = *temp;
    p->topo = p->topo->proximo;
    free(temp);
    return acao;
}

Acao* peek(stack *p) {
    if (p->topo == NULL) {
        printf("Nenhum ataque realizado ainda.\n");
        return NULL;
    }
    return p->topo;
}

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

// Função para verificar se todos os navios de um jogador foram afundados
bool todos_navios_afundados(Jogador *jogador) {
    for (int i = 0; i < MAX_NAVIOS; i++) {
        if (!jogador->navios[i].afundado) {
            return false;
        }
    }
    return true;
}

int main() {
    Node *jogadores = NULL;
    TabelaHash tabela_hash = {0};
    Queue fila_eventos = {0};
    stack pilha_acoes = {0};

    for (int i = 0; i < 2; i++) {
        Jogador novo_jogador;
        novo_jogador.id_jogador = i + 1;
        for (int x = 0; x < TAMANHO_TABULEIRO; x++) {
            for (int y = 0; y < TAMANHO_TABULEIRO; y++) {
                novo_jogador.tabuleiro[x][y] = '-';
                novo_jogador.tabuleiro_visivel[x][y] = '.';
            }
        }
        for (int j = 0; j < MAX_NAVIOS; j++) {
            novo_jogador.navios[j].afundado = false;  // Inicializando navios não afundados
        }
        Node *novo_node = criar_node(novo_jogador);
        if (!jogadores) {
            jogadores = novo_node;
            jogadores->proximo = jogadores;
        } else {
            novo_node->proximo = jogadores->proximo;
            jogadores->proximo = novo_node;
        }
    }

    bool jogo_terminado = false;
    Node *jogador_atual = jogadores;
    while (!jogo_terminado) {
        printf("Turno do jogador %d\n", jogador_atual->jogador.id_jogador);
        imprimir_tabuleiro(jogador_atual->jogador.tabuleiro_visivel);

        // Exibir o último ataque realizado
        Acao *ultima_acao = peek(&pilha_acoes);
        if (ultima_acao != NULL) {
            printf("Último ataque em (%d,%d) foi um %s.\n", ultima_acao->posicao.x, ultima_acao->posicao.y, ultima_acao->acerto ? "acerto" : "erro");
        }

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
            printf("Posição já atacada. Escolha outra.\n");
            continue;
        }

        registrar_acerto(&tabela_hash, tiro);
        bool acerto = rand() % 2;  // Simulação de acerto ou erro
        jogador_atual->jogador.tabuleiro_visivel[x][y] = acerto ? 'A' : 'E';

     
        if (acerto) {
         
            jogador_atual->jogador.navios[rand() % MAX_NAVIOS].afundado = true;
        }

        Acao nova_acao = {tiro, acerto};
        enqueue(&fila_eventos, nova_acao);
        push(&pilha_acoes, nova_acao);

        printf("Resultado do tiro: %s\n", acerto ? "Acerto" : "Erro");

        if (todos_navios_afundados(&jogador_atual->jogador)) {
            printf("Jogador %d venceu! Todos os navios adversários foram afundados.\n", jogador_atual->jogador.id_jogador);
            jogo_terminado = true;
            continue;
        }

        jogador_atual = jogador_atual->proximo;
        if (jogador_atual == jogadores) {
            printf("Todos os jogadores jogaram. Nova rodada.\n");
        }
    }

    if (jogo_terminado) {
        print_queue(&fila_eventos); 
        printf("Fim do jogo!\n");
    }
    return 0;
}