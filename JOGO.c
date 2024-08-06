#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TAMANHO_TABULEIRO 10
#define MAX_NAVIOS 5
#define TAMANHO_HASH 100

typedef struct {
    int x;
    int y;
} Posicao;

typedef struct {
    Posicao start;
    Posicao end;
    int size;
    bool is_sunk;
} Navio;

typedef struct {
    int player_id;
    Navio navios[MAX_NAVIOS];
    int hits;
    int misses;
    int navios_afundados;
    char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
    char tabuleiro_visivel[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
} Jogador;

typedef struct Node {
    Jogador jogador;
    struct Node *next;
} Node;

typedef struct Acao {
    Posicao pos;
    bool hit;
    struct Acao *next;
} Acao;

typedef struct EventoNode {
    Acao acao;
    struct EventoNode *next;
} EventoNode;

typedef struct {
    bool hits[TAMANHO_HASH];
} TabelaHash;

typedef struct TiroNode {
    Posicao posicao;
    struct TiroNode *prev;
    struct TiroNode *next;
} TiroNode;

typedef struct {
    TiroNode *head;
    TiroNode *tail;
} ListaTirosAcertados;

// LISTA SIMPLESMENTE ENCADEADA
typedef struct DicaNode {
    char dica[256];
    struct DicaNode *next;
} DicaNode;

DicaNode* criar_dica_node(const char *dica) {
    DicaNode *novo_node = (DicaNode *)malloc(sizeof(DicaNode));
    strcpy(novo_node->dica, dica);
    novo_node->next = NULL;
    return novo_node;
}

void adicionar_dica(DicaNode **head, const char *dica) {
    DicaNode *novo_node = criar_dica_node(dica);
    novo_node->next = *head;
    *head = novo_node;
}

const char* obter_dica_aleatoria(DicaNode *head) {
    if (head == NULL) return "Nenhuma dica disponível.";

    int count = 0;
    DicaNode *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    int random_index = rand() % count;
    current = head;
    for (int i = 0; i < random_index; i++) {
        current = current->next;
    }

    return current->dica;
}

// LISTA CIRCULAR ( INSERIR, REMOVER E PRINTAR)
Node* create_node(Jogador jogador) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->jogador = jogador;
    new_node->next = new_node;
    return new_node;
}

/*void remove_node(Node **head, int player_id) {
    if (*head == NULL) {
        return; // Lista vazia, nada a remover
    }

    Node *current = *head;
    Node *prev = NULL;

    do {
        if (current->jogador.player_id == player_id) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                Node *last = *head;
                while (last->next != *head) {
                    last = last->next;
                }
                if (current == current->next) {
                    *head = NULL;
                } else {
                    *head = current->next;
                    last->next = *head;
                }
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    } while (current != *head);
}*/

/*void print_lista_circular(Node *head) {
    if (head == NULL) {
        printf("Lista está vazia.\n");
        return;
    }

    Node *current = head;
    do {
        printf("Jogador ID: %d\n", current->jogador.player_id);
        current = current->next;
    } while (current != head);
}*/

Acao* create_acao(Posicao pos, bool hit) {
    Acao *new_acao = (Acao *)malloc(sizeof(Acao));
    new_acao->pos = pos;
    new_acao->hit = hit;
    new_acao->next = NULL;
    return new_acao;
}

EventoNode* create_evento_node(Acao acao) {
    EventoNode *new_node = (EventoNode *)malloc(sizeof(EventoNode));
    new_node->acao = acao;
    new_node->next = NULL;
    return new_node;
}

// FUNÇÃO HASHING
int hash_posicao(Posicao pos) {
    return (pos.x * 10 + pos.y) % TAMANHO_HASH;
}

void record_hit(TabelaHash *tabela, Posicao pos) {
    int index = hash_posicao(pos);
    tabela->hits[index] = true;
}

bool check_hit(TabelaHash *tabela, Posicao pos) {
    int index = hash_posicao(pos);
    return tabela->hits[index];
}

TiroNode* criar_tiro_node(Posicao posicao) {
    TiroNode *novo_tiro = (TiroNode *)malloc(sizeof(TiroNode));
    novo_tiro->posicao = posicao;
    novo_tiro->prev = novo_tiro->next = NULL;
    return novo_tiro;
}

// ESTRUTURA DA FILA ( INSERIR, REMOVER E PRINTAR )
typedef struct Queue {
    EventoNode *front;
    EventoNode *rear;
} Queue;

void enqueue(Queue *q, Acao acao) {
    EventoNode *new_node = create_evento_node(acao);
    if (q->rear == NULL) {
        q->front = q->rear = new_node;
        return;
    }
    q->rear->next = new_node;
    q->rear = new_node;
}

/*Acao dequeue(Queue *q) {
    if (q->front == NULL) {
        exit(EXIT_FAILURE);
    }
    EventoNode *temp = q->front;
    Acao acao = temp->acao;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return acao;
}*/

void print_acoes_perdidas(Queue *q) {
    EventoNode *current = q->front;
    printf("Histórico de ações perdidas:\n");
    while (current != NULL) {
        Acao acao = current->acao;
        if (!acao.hit) {
            printf("Tiro em (%d, %d) foi um Miss.\n", acao.pos.x, acao.pos.y);
        }
        current = current->next;
    }
}

// LLISTA DUPLAMENTE ENCADEADA ( INSERIR REMOVER E PRINTAR )
void adicionar_tiro_acertado(ListaTirosAcertados *lista, Posicao posicao) {
    TiroNode *novo_tiro = criar_tiro_node(posicao);
    if (lista->tail == NULL) {
        lista->head = lista->tail = novo_tiro;
    } else {
        lista->tail->next = novo_tiro;
        novo_tiro->prev = lista->tail;
        lista->tail = novo_tiro;
    }
}

void print_acoes_acertadas(ListaTirosAcertados *lista) {
    TiroNode *current = lista->head;
    printf("Histórico de ações acertadas:\n");
    while (current != NULL) {
        printf("Tiro em (%d, %d) foi um Hit.\n", current->posicao.x, current->posicao.y);
        current = current->next;
    }
}

/*void remover_tiro_acertado(ListaTirosAcertados *lista, Posicao posicao) {
    TiroNode *current = lista->head;
    while (current != NULL) {
        if (current->posicao.x == posicao.x && current->posicao.y == posicao.y) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                lista->head = current->next; // Remover cabeça
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            } else {
                lista->tail = current->prev; // Remover cauda
            }
            free(current);
            return;
        }
        current = current->next;
    }
}*/

// ESTRUTURA DA PILHA ( INSERIR, REMOVER E PRINTAR )
typedef struct Stack {
    Acao *top;
} Stack;

void push(Stack *s, Acao acao) {
    Acao *new_acao = create_acao(acao.pos, acao.hit);
    new_acao->next = s->top;
    s->top = new_acao;
}

/*Acao pop(Stack *s) {
    if (s->top == NULL) {
        exit(EXIT_FAILURE);
    }
    Acao *temp = s->top;
    Acao acao = *temp;
    s->top = s->top->next;
    free(temp);
    return acao;
}*/

Acao* peek(Stack *s) {
    if (s->top == NULL) {
        printf("Nenhum ataque realizado ainda.\n");
        return NULL;
    }
    return s->top;
}

// EXIBIR O TABULEIRO
void print_tabuleiro(char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO]) {
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

// FUNÇÃO PARA VEREFICAR SE TODOS NAVIOS FORAM AFUNDADOS
bool are_all_navios_sunk(Jogador *jogador) {
    return jogador->navios_afundados == MAX_NAVIOS;
}

int main() {
    Node *jogadores = NULL;
    TabelaHash tabela_hash = {0};
    Queue fila_eventos = {0};
    Stack pilha_acoes = {0};
    ListaTirosAcertados lista_acertados = {0};
    DicaNode *dicas = NULL;

    
    adicionar_dica(&dicas, "TENTE ATACAR OS CANTOS DO TABULEIRO.");
    adicionar_dica(&dicas, "FOQUE EM LINHAS, SUA CHANCE DE VENCER TENDE A SER MAIOR.");
    adicionar_dica(&dicas, "USE TIROS QUE VOCÊ JÁ ACERTOU COMO REFERENCIA");
    adicionar_dica(&dicas, "PERCEBA A SITUAÇÃO DO SEU ADVERSÁRIO.");
    adicionar_dica(&dicas, "IMPORTANTE LEMBRAR ONDE VOCE JÁ ATACOU.");

    for (int i = 0; i < 2; i++) {
        Jogador novo_jogador;
        novo_jogador.player_id = i + 1;
        novo_jogador.navios_afundados = 0;
        for (int x = 0; x < TAMANHO_TABULEIRO; x++) {
            for (int y = 0; y < TAMANHO_TABULEIRO; y++) {
                novo_jogador.tabuleiro[x][y] = '-';
                novo_jogador.tabuleiro_visivel[x][y] = '.';
            }
        }
        for (int j = 0; j < MAX_NAVIOS; j++) {
            novo_jogador.navios[j].is_sunk = false;
        }
        Node *novo_node = create_node(novo_jogador);
        if (!jogadores) {
            jogadores = novo_node;
            jogadores->next = jogadores;
        } else {
            novo_node->next = jogadores->next;
            jogadores->next = novo_node;
        }
    }

    bool game_over = false;
    Node *jogador_atual = jogadores;
    while (!game_over) {
        printf("Turno do jogador %d\n", jogador_atual->jogador.player_id);
        print_tabuleiro(jogador_atual->jogador.tabuleiro_visivel);

        // Exibe uma dica para o jogador da vez
        printf("--Dica: %s\n", obter_dica_aleatoria(dicas));

        Acao *last_acao = peek(&pilha_acoes);
        if (last_acao != NULL) {
            printf("Último ataque em (%d,%d) foi um %s.\n", last_acao->pos.x, last_acao->pos.y, last_acao->hit ? "ACERTO" : "ERRO");
        }

        int x, y;
        printf("Insira a coordenada X para atirar: ");
        scanf("%d", &x);
        printf("Insira a coordenada Y para atirar: ");
        scanf("%d", &y);

        Posicao tiro = {x, y};
        if (x < 0 || x >= TAMANHO_TABULEIRO || y < 0 || y >= TAMANHO_TABULEIRO) {
            printf("COORDENADA INVÁLIDAS, TENTE NOVAMENTE\n");
            continue;
        }

        if (check_hit(&tabela_hash, tiro)) {
            printf("------A POSIÇÃO ESCOLHIDA JÁ FOI ATACADA, ESCOLHA OUTRA !!!.\n");
            continue;
        }

        record_hit(&tabela_hash, tiro);
        bool hit = rand() % 2;
        jogador_atual->jogador.tabuleiro_visivel[x][y] = hit ? 'H' : 'M';

        if (hit) {
            jogador_atual->jogador.navios[rand() % MAX_NAVIOS].is_sunk = true;
            jogador_atual->jogador.navios_afundados++;
            adicionar_tiro_acertado(&lista_acertados, tiro);
        }

        Acao nova_acao = {tiro, hit};
        if (!hit) {
            enqueue(&fila_eventos, nova_acao);
        }
        push(&pilha_acoes, nova_acao);

        printf("Resultado do tiro: %s\n", hit ? "Hit" : "Miss");

        if (are_all_navios_sunk(&jogador_atual->jogador)) {
            printf("Jogador %d venceu! Todos os navios adversários foram afundados.\n", jogador_atual->jogador.player_id);
            game_over = true;
            continue;
        }

        jogador_atual = jogador_atual->next;
        if (jogador_atual == jogadores) {
            printf("Todos os jogadores jogaram. Nova rodada.\n");
        }
    }

    if (game_over) {
         printf("----------------------------------------");
        print_acoes_perdidas(&fila_eventos);
         printf("----------------------------------------");
        print_acoes_acertadas(&lista_acertados);
        printf("Fim do jogo!\n");
    }

    return 0;
}