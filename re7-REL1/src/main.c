#include "raylib.h"

#include "game.h"
#include "assets.h"
#include "audio_manager.h"
#include "ui.h"
#include "barra_tokens.h"
#include "senha.h"
#include "dev_mode.h"       // modo desenvolvedor: so no build -Dev (senao nao faz nada)

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
        DevMode_Atualizar(&game);

        // um clique no painel de desenvolvedor nao chega ao jogo
        if (!DevMode_CapturouEntrada())
            UpdateGame(&game);

        BeginDrawing();

        ClearBackground(BLACK);

        DrawGame(&game, &assets);
        DrawPauseControls(&game);
        DevMode_Desenhar(&game);

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

Build de desenvolvimento, com o painel de testes (F10):

.\build.ps1 -Dev
.\re7_dev.exe

*/