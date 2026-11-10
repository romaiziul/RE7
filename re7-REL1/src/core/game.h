#ifndef GAME_H
#define GAME_H

#include "raylib.h"

#include "assets.h"
#include "scenes.h"


// Qual tela do jogo está aberta?

typedef enum
{
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_CREDITS,
    SCREEN_MINIGAME,        // minijogo antigo (nao usado mais pelo fluxo)
    SCREEN_ENDING,
    SCREEN_ROOM,
    SCREEN_AUDIO,
    SCREEN_SENHA,           // Ato 1: minijogo da senha
    SCREEN_CORRIDA,         // Ato 2: corrida de obstaculos
    SCREEN_PAUSE,
    SCREEN_CONFIRM_MENU,
    SCREEN_ENTRE_LINHAS     // Ato 3: desafio "Entre as Linhas"
} GameScreen;


// Como a campanha terminou (ou se ainda esta em andamento)

typedef enum
{
    CAMPANHA_EM_ANDAMENTO,
    CAMPANHA_FINAL_BOM,         // epilogo
    CAMPANHA_FINAL_RUIM
} EstadoCampanha;

// Por que o final ruim aconteceu (muda o cartao da tela final)

typedef enum
{
    FINAL_RUIM_IA_LEU,          // a IA leu o papel no Ato 3
    FINAL_RUIM_SEM_TOKENS       // pediu ajuda e a barra chegou a 0
} MotivoFinalRuim;


// Tudo que precisamos saber
// sobre o estado atual do jogo.

typedef struct
{
    GameScreen currentScreen;

    SceneID currentScene;

    int dialogueIndex;

    GameScreen pauseReturnScreen;

    int choice;             // escolha na tela (0 = nenhuma); ver EscolhaId no game.c

    bool senhaNoQuarto;     // true: a Lia voltou ao quarto depois de 5 erros na senha

    bool iaEscreveuDiscurso;    // Ato 2: a jogadora deixou a IA escrever o discurso

    EstadoCampanha campanha;

    MotivoFinalRuim motivoFinalRuim;

    bool shouldExit;

} GameState;


// Funções principais

void InitGame(GameState *game);

void UpdateGame(GameState *game);

void DrawPauseControls(GameState *game);

void DrawGame(
    GameState *game,
    Assets *assets
);

void ChangeScene(
    GameState *game,
    SceneID nextScene
);

// =====================================================
// API do modo desenvolvedor (src/dev). So existe no build de
// desenvolvimento (.\build.ps1 -Dev). Reaproveita a logica da campanha
// do game.c: nada aqui reimplementa regras.
// =====================================================

#ifdef RE7_DEV_MODE

typedef enum
{
    ATO_1,
    ATO_2,
    ATO_3,
    ATO_EPILOGO
} AtoId;

AtoId Game_AtoAtual(const GameState *game);

// Vai direto para o inicio do ato, com os tokens e variaveis atuais.
void Game_IrParaAto(GameState *game, AtoId ato);

// Volta ao inicio do ato atual, com os tokens e variaveis da entrada no ato.
void Game_ReiniciarAto(GameState *game);

// Igual ao "Novo jogo" do menu.
void Game_ReiniciarCampanha(GameState *game);

// Pula a fala (ou animacao) atual. Devolve false se ha uma escolha na tela.
bool Game_PularFala(GameState *game);

// Pula o resto da cena ou conclui o minijogo atual. False se ha uma escolha.
bool Game_AvancarEtapa(GameState *game);

// Vai direto para a tela final (bom ou ruim).
void Game_ForcarFinal(GameState *game, EstadoCampanha final);

const char *Game_NomeDaCena(SceneID cena);
const char *Game_NomeDaTela(GameScreen tela);

#endif

#endif