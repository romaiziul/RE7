// teste_senha.c
// Programa de teste do minijogo da senha, separado do jogo.
// Mostra a barra de tokens no canto. No jogo, quem desenha a barra e o nucleo.
// Rode a partir da pasta re7-REL1, para a fonte Nunito ser encontrada em assets/fonts.
//
// Compilar (Windows, w64devkit), dentro de re7-REL1:
//   gcc testes/teste_senha.c src/atos/ato1/senha.c src/systems/barra_tokens.c src/systems/tokens.c src/engine/ui.c -Isrc -Isrc/atos/ato1 -Isrc/systems -Isrc/engine -Isrc/core -IC:/raylib/raylib/src -LC:/raylib/raylib/src -o teste_senha.exe -lraylib -lopengl32 -lgdi32 -lwinmm
// Compilar (macOS), dentro de re7-REL1:
//   cc testes/teste_senha.c src/atos/ato1/senha.c src/systems/barra_tokens.c src/systems/tokens.c src/engine/ui.c -Isrc -Isrc/atos/ato1 -Isrc/systems -Isrc/engine -Isrc/core -o teste_senha $(pkg-config --cflags --libs raylib)
//
// Etapa 1: clique nos caracteres e na seta verde (ou ENTER).
// Etapa 2: arraste os caracteres para as posicoes. Seta verde (ou ENTER) confirma.
// Na janela da IA: clique ou 1 / 2.
// Depois de acertar: R reinicia.

#include "raylib.h"
#include "senha.h"
#include "tokens.h"
#include "barra_tokens.h"
#include "ui.h"

int main(void)
{
    InitWindow(1280, 720, "re:7 - teste da senha (Ato 1)");
    SetTargetFPS(60);
    UiLoadFonts();

    Tokens_Reiniciar();
    BarraTokens_Reiniciar();
    Senha_ComecarJogo();

    while (!WindowShouldClose()) {
        Senha_Atualizar();
        BarraTokens_Atualizar();

        if (Senha_Concluida() && IsKeyPressed(KEY_R)) {
            Tokens_Reiniciar();
            BarraTokens_Reiniciar();
            Senha_ComecarJogo();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        Senha_Desenhar();
        BarraTokens_Desenhar(990, 20);
        if (Senha_Concluida()) {
            UiDrawText("Teste concluído. R reinicia.", 20, 690, 20, RAYWHITE);
        }
        EndDrawing();
    }

    Senha_Descarregar();
    BarraTokens_Descarregar();
    UiUnloadFonts();
    CloseWindow();
    return 0;
}
