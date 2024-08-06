#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define TAMANHO_TABULEIRO 10
#define MAX_NAVIOS 4
#define TAMANHO_HASH 100
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 600
#define CELL_SIZE (WINDOW_WIDTH / TAMANHO_TABULEIRO)
#define DICA_INTERVAL 5000 // Intervalo de 5 segundos para troca de dicas

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

typedef struct DicaNode {
    char dica[256];
    struct DicaNode *next;
} DicaNode;



// LISTA SIMPLESMENTE ENCADEADA (INSERIR, REMOVER, PRINTAR)
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

/*
void remover_dica(DicaNode **head, const char *dica) {
    if (*head == NULL) return;

    DicaNode *temp = *head, *prev = NULL;

    // Se o nó a ser removido é o primeiro nó
    if (temp != NULL && strcmp(temp->dica, dica) == 0) {
        *head = temp->next; // Muda o head
        free(temp);         // Libera a memória do antigo head
        return;
    }

    // Procura pelo nó a ser removido, mantendo o track do nó anterior
    while (temp != NULL && strcmp(temp->dica, dica) != 0) {
        prev = temp;
        temp = temp->next;
    }

    // Se a dica não foi encontrada
    if (temp == NULL) return;

    // Desconecta o nó da lista
    prev->next = temp->next;

    free(temp); // Libera a memória do nó removido
}
*/


// LISTA CIRCULAR (INSERIR, REMOVER E PRINTAR)
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



// FUNÇÃO HASH
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

// node cirar tiro
TiroNode* criar_tiro_node(Posicao posicao) {
    TiroNode *novo_tiro = (TiroNode *)malloc(sizeof(TiroNode));
    novo_tiro->posicao = posicao;
    novo_tiro->prev = novo_tiro->next = NULL;
    return novo_tiro;
}


//FILA(INSERIR, REMVOER E PRINTAR)
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

//LISTA DUPLAMENTE ENCADEADA
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

// PILHA(INSERIR REMVOER E PRINTAR)
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

void desenhar_tabuleiro(SDL_Renderer *renderer, char tabuleiro[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO]) {
    SDL_Color preto = {0, 0, 0, 255};
    SDL_Color vermelho = {255, 0, 0, 255};
    SDL_Color azul = {0, 0, 255, 255};
    SDL_Color branco = {255, 255, 255, 255};

    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            SDL_Rect cell = { i * CELL_SIZE, j * CELL_SIZE + 50, CELL_SIZE, CELL_SIZE }; // o 50 auxilia na margem(renderização)
            if (tabuleiro[i][j] == 'H') {
                SDL_SetRenderDrawColor(renderer, vermelho.r, vermelho.g, vermelho.b, vermelho.a); // Vermelho para acertos
            } else if (tabuleiro[i][j] == 'M') {
                SDL_SetRenderDrawColor(renderer, azul.r, azul.g, azul.b, azul.a); // Azul para erros
            } else {
                SDL_SetRenderDrawColor(renderer, branco.r, branco.g, branco.b, branco.a); // Branco para não atacado
            }
            SDL_RenderFillRect(renderer, &cell);
            SDL_SetRenderDrawColor(renderer, preto.r, preto.g, preto.b, preto.a); // Preto para linhas de grade
            SDL_RenderDrawRect(renderer, &cell);
        }
    }
}

void render_menu(SDL_Renderer *renderer, TTF_Font *font) {
    // Limpa a tela
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Renderiza botão "Começar o Jogo"
    SDL_Rect startButton = {100, 100, 300, 100};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Verde
    SDL_RenderFillRect(renderer, &startButton);

    // Renderiza botão "Sair"
    SDL_Rect quitButton = {100, 300, 300, 100};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Vermelho
    SDL_RenderFillRect(renderer, &quitButton);

    // Desenha bordas pretas dos botões
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &startButton);
    SDL_RenderDrawRect(renderer, &quitButton);

    // Renderiza texto nos botões
    SDL_Color preto = {0, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderUTF8_Solid(font, "Começar o Jogo", preto);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {150, 125, 200, 50};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    surface = TTF_RenderUTF8_Solid(font, "Sair", preto);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    textRect = (SDL_Rect){200, 325, 100, 50};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Atualiza a tela
    SDL_RenderPresent(renderer);
}

void render_info(SDL_Renderer *renderer, TTF_Font *font, const char *info, int x, int y) { // responsavel por estruturar o texto 
    SDL_Color preto = {0, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderUTF8_Solid(font, info, preto);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void render_vitoria(SDL_Renderer *renderer, TTF_Font *font, int player_id, Queue *fila_eventos, ListaTirosAcertados *lista_acertados) {
    // Limpa a tela
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Ajusta o tamanho da fonte para o texto final
    TTF_Font *final_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    if (!final_font) {
        printf("Erro ao carregar fonte: %s\n", TTF_GetError());
        return;
    }

    // Renderiza mensagem de vitória
    SDL_Color preto = {0, 0, 0, 255};
    char mensagem[256];
    snprintf(mensagem, sizeof(mensagem), "Jogador %d venceu!", player_id);
    SDL_Surface *surface = TTF_RenderUTF8_Solid(final_font, mensagem, preto);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {50, 50, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Renderiza ações perdidas
    int y_offset = 150;
    surface = TTF_RenderUTF8_Solid(final_font, "Histórico de Ações Perdidas:", preto);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    textRect = (SDL_Rect){50, y_offset, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    y_offset += 30;

    EventoNode *current_event = fila_eventos->front;
    while (current_event != NULL) {
        Acao acao = current_event->acao;
        if (!acao.hit) {
            snprintf(mensagem, sizeof(mensagem), "Tiro em (%d, %d) foi um Miss.", acao.pos.x, acao.pos.y);
            surface = TTF_RenderUTF8_Solid(final_font, mensagem, preto);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            textRect = (SDL_Rect){50, y_offset, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, NULL, &textRect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
            y_offset += 30;
        }
        current_event = current_event->next;
    }

    // Renderiza ações acertadas
    y_offset += 30;
    surface = TTF_RenderUTF8_Solid(final_font, "Histórico de Ações Acertadas:", preto);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    textRect = (SDL_Rect){50, y_offset, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    y_offset += 30;

    TiroNode *current_tiro = lista_acertados->head;
    while (current_tiro != NULL) {
        snprintf(mensagem, sizeof(mensagem), "Tiro em (%d, %d) foi um Hit.", current_tiro->posicao.x, current_tiro->posicao.y);
        surface = TTF_RenderUTF8_Solid(final_font, mensagem, preto);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        textRect = (SDL_Rect){50, y_offset, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &textRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        y_offset += 30;
        current_tiro = current_tiro->next;
    }

    // Atualiza a tela
    SDL_RenderPresent(renderer);

    // Libera a fonte
    TTF_CloseFont(final_font);
}

bool are_all_navios_sunk(Jogador *jogador) {
    return jogador->navios_afundados == MAX_NAVIOS;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Erro ao inicializar TTF: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Altere o caminho para uma fonte que esteja disponível no seu sistema
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!font) {
        printf("Erro ao carregar fonte: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Batalha Naval",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Erro ao criar renderizador: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Node *jogadores = NULL;
    TabelaHash tabela_hash = {0};
    Queue fila_eventos = {0};
    Stack pilha_acoes = {0};
    ListaTirosAcertados lista_acertados = {0};
    DicaNode *dicas = NULL;

    adicionar_dica(&dicas, "Ataque os cantos.");
    adicionar_dica(&dicas, "Foque em linhas.");
    adicionar_dica(&dicas, "Use acertos como referência.");
    adicionar_dica(&dicas, "Observe o adversário.");
    adicionar_dica(&dicas, "Lembre onde já atacou.");

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
    bool running = true;
    bool in_game = false;
    const char *dica_atual = "Nenhuma dica disponível.";
    Uint32 ultimo_tempo_dica = SDL_GetTicks();
    int vencedor_id = -1;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // variavel que armazena os eventos capturados 
            if (event.type == SDL_QUIT) { // se todos os eventos acontecerem, o jogo termina  ( o runnig deixa de correr)
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && !in_game) { // essa parte ve quando nao esta rolando o jogo, ele armazena o click do mouse para ve se esta dentro do limite de começar ou sair do jogo
                int x = event.button.x;
                int y = event.button.y;

                if (x >= 100 && x <= 400 && y >= 100 && y <= 200) {
                    in_game = true;
                } else if (x >= 100 && x <= 400 && y >= 300 && y <= 400) {
                    running = false;
                } 
            } else if (event.type == SDL_MOUSEBUTTONDOWN && in_game) { //pega as posições do click e ve se ta dentro do tabuleiro e pega a posição do tiro
                int x = (event.button.x - 10) / CELL_SIZE;
                int y = (event.button.y - 50) / CELL_SIZE;

                if (x < 0 || x >= TAMANHO_TABULEIRO || y < 0 || y >= TAMANHO_TABULEIRO) {
                    continue;
                }

                Posicao tiro = {x, y};

                if (check_hit(&tabela_hash, tiro)) {
                    printf("Posição já atacada. Escolha outra.\n");
                    continue;
                }

                record_hit(&tabela_hash, tiro);
                bool hit = rand() % 2;
                jogador_atual->jogador.tabuleiro_visivel[tiro.x][tiro.y] = hit ? 'H' : 'M';

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
                    vencedor_id = jogador_atual->jogador.player_id;
                    game_over = true;
                    in_game = false;
                }

                jogador_atual = jogador_atual->next;
                if (jogador_atual == jogadores) {
                    printf("Todos os jogadores jogaram. Nova rodada.\n");
                }
            }
        }

        Uint32 tempo_atual = SDL_GetTicks();
        if (tempo_atual - ultimo_tempo_dica > DICA_INTERVAL) {
            dica_atual = obter_dica_aleatoria(dicas);
            ultimo_tempo_dica = tempo_atual;
        }

        if (!in_game) {
            if (game_over) {
                render_vitoria(renderer, font, vencedor_id, &fila_eventos, &lista_acertados);
            } else {
                render_menu(renderer, font);
            }
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Limpa a tela com branco
            SDL_RenderClear(renderer);

            // Renderiza o tabuleiro
            desenhar_tabuleiro(renderer, jogador_atual->jogador.tabuleiro_visivel);

            // Renderiza a última posição atacada
            Acao *last_acao = peek(&pilha_acoes);
            char info[256];
            if (last_acao != NULL) {
                snprintf(info, sizeof(info), "Último ataque em (%d, %d) foi um %s.", last_acao->pos.x, last_acao->pos.y, last_acao->hit ? "acerto" : "erro");
                render_info(renderer, font, info, 10, 10);
            }

            // Renderiza a dica atual
            snprintf(info, sizeof(info), "Dica: %s", dica_atual);
            render_info(renderer, font, info, 10, WINDOW_HEIGHT - 40);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}