#include "raylib.h"
#include <math.h>

#include "game.h"
#include "assets.h"
#include "audio_manager.h"
#include "ui.h"
#include "barra_tokens.h"
#include "senha.h"
#include "tela.h"
#include "dev_mode.h"       // modo desenvolvedor: so no build -Dev (senao nao faz nada)

int main(void)
{
    const int screenWidth = TELA_LARGURA;
    const int screenHeight = TELA_ALTURA;

    // A janela pode mudar de tamanho; F11 alterna a tela cheia.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

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

    // O jogo desenha sempre em 1280x720 nesta textura. No fim de cada quadro,
    // ela e ampliada para a janela mantendo a proporcao (sobras ficam pretas).
    RenderTexture2D quadro = LoadRenderTexture(TELA_LARGURA, TELA_ALTURA);
    SetTextureFilter(quadro.texture, TEXTURE_FILTER_BILINEAR);


    while (!WindowShouldClose() && !game.shouldExit)
    {
        // F11: tela cheia sem bordas, sem trocar a resolucao do monitor
        if (IsKeyPressed(KEY_F11))
            ToggleBorderlessWindowed();

        // escala que cabe na janela sem distorcer, e o quadro centralizado
        float escala = fminf(
            (float)GetScreenWidth() / TELA_LARGURA,
            (float)GetScreenHeight() / TELA_ALTURA
        );
        float margemX = ((float)GetScreenWidth() - TELA_LARGURA * escala) / 2.0f;
        float margemY = ((float)GetScreenHeight() - TELA_ALTURA * escala) / 2.0f;

        // o mouse continua em coordenadas de 1280x720 para o resto do jogo
        SetMouseOffset((int)-margemX, (int)-margemY);
        SetMouseScale(1.0f / escala, 1.0f / escala);

        DevMode_Atualizar(&game);

        // um clique no painel de desenvolvedor nao chega ao jogo
        if (!DevMode_CapturouEntrada())
            UpdateGame(&game);

        BeginTextureMode(quadro);

        ClearBackground(BLACK);

        DrawGame(&game, &assets);
        DrawPauseControls(&game);
        DevMode_Desenhar(&game);

        EndTextureMode();

        BeginDrawing();

        ClearBackground(BLACK);   // faixas pretas quando a proporcao e diferente

        // altura negativa: a textura fica de cabeca para baixo na memoria
        DrawTexturePro(
            quadro.texture,
            (Rectangle){ 0, 0, (float)TELA_LARGURA, -(float)TELA_ALTURA },
            (Rectangle){ margemX, margemY, TELA_LARGURA * escala, TELA_ALTURA * escala },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );

        EndDrawing();
    }


    UnloadRenderTexture(quadro);
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