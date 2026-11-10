#include "raylib.h"

#include "game.h"
#include "assets.h"
#include "audio_manager.h"
#include "ui.h"
#include "barra_tokens.h"
#include "senha.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(
        screenWidth,
        screenHeight,
        "Re_7"
    );

    UiLoadFonts();
    AudioManager_Init();

    SetTargetFPS(60);

    // ESC e tratado pelas telas do jogo.
    SetExitKey(KEY_NULL);

    Assets assets;

    LoadAssets(&assets);

    GameState game;

    InitGame(&game);


    while (!WindowShouldClose() && !game.shouldExit)
    {
        UpdateGame(&game);

        BeginDrawing();

        ClearBackground(BLACK);

        DrawGame(&game, &assets);
        DrawPauseControls(&game);

        EndDrawing();
    }


    UnloadAssets(&assets);
    BarraTokens_Descarregar();
    Senha_Descarregar();

    AudioManager_Close();

    UiUnloadFonts();
    CloseWindow();

    return 0;
}

/* Para compilar, na raiz do projeto (re7-REL1):

.\build.ps1

Para rodar (sempre da raiz do projeto):

.\re7.exe

*/