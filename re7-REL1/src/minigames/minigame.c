/*
 * Minigame de obstaculos (adaptado do Mini-game-3-C).
 *
 * Fluxo: corrida de obstaculos -> (900 pontos) -> vitoria.
 *
 * Controles:
 *   Corrida: ESPACO, W ou seta para cima para pular
 *   Fim:     ENTER joga de novo, ESC volta ao menu
 *   Vitoria: ENTER continua a historia
 *   ESC em qualquer tela do minigame volta ao menu do jogo.
 *
 * O minigame foi feito para uma janela de 800x450. Ele continua desenhando
 * nessas medidas, e a camera amplia tudo para a janela do jogo (1280x720).
 */

#include "minigame.h"
#include "audio_manager.h"

#include "raylib.h"
#include "ui.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define LARGURA          800
#define ALTURA           450
#define CHAO_Y           360

#define JOG_X            100.0f
#define JOG_LARGURA      40.0f
#define JOG_ALTURA       50.0f

#define GRAVIDADE        2200.0f
#define FORCA_PULO       780.0f
#define VEL_INICIAL      400.0f
#define VEL_MAXIMA       920.0f
#define ACELERACAO       20.0f

#define MAX_OBSTACULOS   8
#define BONUS_OBSTACULO  25.0f
#define PONTOS_POR_SEG   10.0f

#define ARQUIVO_RECORDE  "recorde.dat"

#define PONTOS_TRANSICAO    900.0f

/* Qual tela do minigame esta ativa */
typedef enum { TELA_JOGO, TELA_FIM, TELA_VITORIA } Tela;
/* O que aconteceu na corrida neste quadro */
typedef enum { RESULTADO_NADA, RESULTADO_COLISAO, RESULTADO_TRANSICAO } ResultadoCorrida;

/* Jogador da corrida: corpo (posicao e tamanho), velocidade vertical e se esta no chao */
typedef struct {
    Rectangle corpo;
    float velY;
    bool noChao;
} Jogador;

/* Obstaculo da corrida; "contado" evita dar o bonus de pontos duas vezes */
typedef struct {
    Rectangle corpo;
    bool ativo;
    bool contado;
} Obstaculo;

/* Estado completo da corrida: jogador, obstaculos, velocidade, pontos e chao */
typedef struct {
    Jogador jogador;
    Obstaculo obstaculos[MAX_OBSTACULOS];
    float velocidade;
    float tempoSpawn;
    float pontos;
    float deslocamentoChao;
} Jogo;

/* ---------- Estado do minigame (fica guardado entre os quadros) ---------- */

/* Tela atual do minigame */
static Tela tela = TELA_JOGO;
/* Estado da corrida de obstaculos */
static Jogo jogo;
/* Melhor pontuacao ja feita (lida de recorde.dat) */
static int recorde = 0;
/* Pontos da ultima corrida, mostrados na tela de fim */
static int pontuacaoFinal = 0;
/* true se a ultima corrida bateu o recorde */
static bool novoRecorde = false;

/* ---------- Recorde ---------- */

/* Le o recorde do arquivo; devolve 0 se o arquivo nao existir */
static int CarregarRecorde(void)
{
    int valor = 0;
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "rb");
    if (arquivo != NULL) {
        if (fread(&valor, sizeof(int), 1, arquivo) != 1) valor = 0;
        fclose(arquivo);
    }
    return valor;
}

/* Grava o recorde no arquivo */
static void SalvarRecorde(int valor)
{
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "wb");
    if (arquivo != NULL) {
        fwrite(&valor, sizeof(int), 1, arquivo);
        fclose(arquivo);
    }
}

/* ---------- Logica da corrida ---------- */

/* Volta a corrida ao estado inicial (pontos zerados, sem obstaculos) */
static void ReiniciarJogo(Jogo *partida)
{
    partida->jogador.corpo = (Rectangle){ JOG_X, CHAO_Y - JOG_ALTURA, JOG_LARGURA, JOG_ALTURA };
    partida->jogador.velY = 0.0f;
    partida->jogador.noChao = true;

    for (int i = 0; i < MAX_OBSTACULOS; i++) partida->obstaculos[i].ativo = false;

    partida->velocidade = VEL_INICIAL;
    partida->tempoSpawn = 1.2f;
    partida->pontos = 0.0f;
    partida->deslocamentoChao = 0.0f;
}

/* Coloca um novo obstaculo, de tamanho aleatorio, na borda direita da tela */
static void CriarObstaculo(Jogo *partida)
{
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (!partida->obstaculos[i].ativo) {
            float largura = (float)GetRandomValue(24, 50);
            float altura = (float)GetRandomValue(40, 90);
            partida->obstaculos[i].corpo = (Rectangle){ LARGURA + 20.0f, CHAO_Y - altura, largura, altura };
            partida->obstaculos[i].ativo = true;
            partida->obstaculos[i].contado = false;
            return;
        }
    }
}

/* Retorna o resultado do frame: colisao, fim do desafio (pontos atingidos), ou nada. */
static ResultadoCorrida AtualizarJogo(Jogo *partida, float tempoQuadro)
{
    Jogador *jogador = &partida->jogador;

    bool pediuPulo = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    if (pediuPulo && jogador->noChao) {
        jogador->velY = -FORCA_PULO;
        jogador->noChao = false;
    }

    jogador->velY += GRAVIDADE * tempoQuadro;
    jogador->corpo.y += jogador->velY * tempoQuadro;
    if (jogador->corpo.y >= CHAO_Y - jogador->corpo.height) {
        jogador->corpo.y = CHAO_Y - jogador->corpo.height;
        jogador->velY = 0.0f;
        jogador->noChao = true;
    }

    partida->velocidade += ACELERACAO * tempoQuadro;
    if (partida->velocidade > VEL_MAXIMA) partida->velocidade = VEL_MAXIMA;

    partida->pontos += PONTOS_POR_SEG * tempoQuadro;
    partida->deslocamentoChao += partida->velocidade * tempoQuadro;
    if (partida->deslocamentoChao >= 60.0f) partida->deslocamentoChao -= 60.0f;

    partida->tempoSpawn -= tempoQuadro;
    if (partida->tempoSpawn <= 0.0f) {
        CriarObstaculo(partida);
        partida->tempoSpawn = 1.0f + (float)GetRandomValue(0, 100) / 100.0f;
    }

    Rectangle hitbox = { jogador->corpo.x + 6, jogador->corpo.y + 6, jogador->corpo.width - 12, jogador->corpo.height - 8 };

    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        Obstaculo *obstaculo = &partida->obstaculos[i];
        if (!obstaculo->ativo) continue;

        obstaculo->corpo.x -= partida->velocidade * tempoQuadro;

        if (obstaculo->corpo.x + obstaculo->corpo.width < 0.0f) {
            obstaculo->ativo = false;
            continue;
        }

        if (!obstaculo->contado && obstaculo->corpo.x + obstaculo->corpo.width < jogador->corpo.x) {
            obstaculo->contado = true;
            partida->pontos += BONUS_OBSTACULO;
        }

        if (CheckCollisionRecs(hitbox, obstaculo->corpo)) return RESULTADO_COLISAO;
    }

    if (partida->pontos >= PONTOS_TRANSICAO) return RESULTADO_TRANSICAO;

    return RESULTADO_NADA;
}

/* ---------- Desenho da corrida ---------- */

/* Escreve um texto centralizado na largura do minigame */
static void TextoCentral(const char *texto, int y, int tamanho, Color cor)
{
    UiDrawText(texto, (LARGURA - UiMeasureText(texto, tamanho)) / 2, y, tamanho, cor);
}

/* Desenha o fundo e o chao, que "anda" conforme o deslocamento */
static void DesenharCenario(float deslocamento)
{
    ClearBackground((Color){ 24, 24, 38, 255 });
    DrawRectangle(0, CHAO_Y, LARGURA, ALTURA - CHAO_Y, (Color){ 46, 46, 70, 255 });
    DrawRectangle(0, CHAO_Y, LARGURA, 3, RAYWHITE);

    int inicio = -(int)deslocamento;
    for (int x = inicio; x < LARGURA; x += 60) {
        DrawRectangle(x, CHAO_Y + 25, 24, 4, (Color){ 90, 90, 120, 255 });
    }
}

/* Desenha os obstaculos e o jogador */
static void DesenharObjetos(const Jogo *partida)
{
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        const Obstaculo *obstaculo = &partida->obstaculos[i];
        if (!obstaculo->ativo) continue;
        DrawRectangleRec(obstaculo->corpo, (Color){ 230, 80, 80, 255 });
        DrawRectangleLinesEx(obstaculo->corpo, 2, MAROON);
    }

    const Rectangle *corpoJogador = &partida->jogador.corpo;
    DrawRectangleRec(*corpoJogador, SKYBLUE);
    DrawRectangleLinesEx(*corpoJogador, 2, BLUE);
    DrawRectangle((int)corpoJogador->x + 24, (int)corpoJogador->y + 10, 10, 10, WHITE);
    DrawRectangle((int)corpoJogador->x + 29, (int)corpoJogador->y + 13, 5, 5, BLACK);
}

/* Desenha os pontos (esquerda) e o recorde (direita) */
static void DesenharHud(const Jogo *partida, int melhor)
{
    UiDrawText(TextFormat("Pontos: %d", (int)partida->pontos), 44, 16, 24, RAYWHITE);

    const char *textoRecorde = TextFormat("Recorde: %d", melhor);
    UiDrawText(textoRecorde, LARGURA - UiMeasureText(textoRecorde, 24) - 20, 16, 24, GOLD);
}

/* Tela de fim de jogo: pontos, recorde e opcoes */
static void DesenharFim(int pontuacao, int melhor, bool ehNovoRecorde)
{
    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.6f));
    TextoCentral("FIM DE JOGO", 100, 50, RAYWHITE);
    TextoCentral(TextFormat("Pontos: %d", pontuacao), 180, 30, RAYWHITE);

    if (ehNovoRecorde) {
        TextoCentral("Novo recorde!", 225, 26, GOLD);
    } else {
        TextoCentral(TextFormat("Recorde: %d", melhor), 225, 26, LIGHTGRAY);
    }

    TextoCentral("ENTER: jogar de novo    ESC: menu", 300, 20, LIGHTGRAY);
}

/* =====================================================
   FUNCOES PUBLICAS (usadas pelo game.c)
   ===================================================== */

/* Prepara o minigame para comecar do zero */
void InitMinigame(void)
{
    // Inicia a musica de aventura do minigame
    AudioManager_PlayMusic(MUSICA_AVENTURA);
    
    SetRandomSeed((unsigned int)time(NULL));

    recorde = CarregarRecorde();
    pontuacaoFinal = 0;
    novoRecorde = false;

    ReiniciarJogo(&jogo);

    tela = TELA_JOGO;
}


/* Atualiza o minigame e informa se continua, volta ao menu ou foi vencido */
MinigameResult UpdateMinigame(void)
{
    float tempoQuadro = GetFrameTime();
    if (tempoQuadro > 0.05f) tempoQuadro = 0.05f;

    switch (tela) {

    case TELA_JOGO: {
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;

        ResultadoCorrida resultado = AtualizarJogo(&jogo, tempoQuadro);

        if (resultado == RESULTADO_COLISAO) {
            pontuacaoFinal = (int)jogo.pontos;
            novoRecorde = pontuacaoFinal > recorde;
            if (novoRecorde) {
                recorde = pontuacaoFinal;
                SalvarRecorde(recorde);
            }
            tela = TELA_FIM;
        } else if (resultado == RESULTADO_TRANSICAO) {
            tela = TELA_VITORIA;

            // Inicia a musica de vitoria
            AudioManager_PlayMusic(MUSICA_VITORIA);
        }
        break;
    }

    case TELA_FIM:
        if (IsKeyPressed(KEY_ENTER)) {
            ReiniciarJogo(&jogo);
            tela = TELA_JOGO;
        }
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        break;

    case TELA_VITORIA:
        if (IsKeyPressed(KEY_ENTER)) return MINIGAME_VENCEU;
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        break;
    }

    return MINIGAME_CONTINUA;
}


/* Desenha o minigame ampliado (camera) para caber na janela do jogo */
void DrawMinigame(void)
{
    // O minigame foi desenhado em 800x450: a camera amplia para a janela.
    float zoomX = (float)GetScreenWidth() / (float)LARGURA;
    float zoomY = (float)GetScreenHeight() / (float)ALTURA;

    Camera2D camera = { 0 };
    camera.zoom = (zoomX < zoomY) ? zoomX : zoomY;

    BeginMode2D(camera);

    switch (tela) {

    case TELA_JOGO:
        DesenharCenario(jogo.deslocamentoChao);
        DesenharObjetos(&jogo);
        DesenharHud(&jogo, recorde);
        break;

    case TELA_FIM:
        DesenharCenario(jogo.deslocamentoChao);
        DesenharObjetos(&jogo);
        DesenharHud(&jogo, recorde);
        DesenharFim(pontuacaoFinal, recorde, novoRecorde);
        break;


    case TELA_VITORIA:
        ClearBackground((Color){ 18, 18, 30, 255 });
        TextoCentral("REDE RESTAURADA!", 160, 40, GOLD);
        TextoCentral("Voce atravessou a rede.", 220, 24, RAYWHITE);
        TextoCentral("ENTER: continuar    ESC: menu", 290, 20, LIGHTGRAY);
        break;
    }

    EndMode2D();
}