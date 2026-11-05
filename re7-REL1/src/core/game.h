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
    SCREEN_CORRIDA          // Ato 2: corrida de obstaculos
} GameScreen;


// Tudo que precisamos saber
// sobre o estado atual do jogo.

typedef struct
{
    GameScreen currentScreen;

    SceneID currentScene;

    int dialogueIndex;

    GameScreen audioReturnScreen;

    int choice;             // escolha na tela (0 = nenhuma); ver EscolhaId no game.c

    bool senhaNoQuarto;     // true: a Lia voltou ao quarto depois de 5 erros na senha

    bool shouldExit;

} GameState;


// Funções principais

void InitGame(GameState *game);

void UpdateGame(GameState *game);

void DrawAudioButton(GameState *game);

void DrawGame(
    GameState *game,
    Assets *assets
);

void ChangeScene(
    GameState *game,
    SceneID nextScene
);

#endif