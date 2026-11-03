// teste_senha.c
// Programa de teste do minijogo da senha, separado do jogo.
// Mostra a barra de tokens no canto. No jogo, quem desenha a barra e o nucleo.
//
// Compilar (Windows, w64devkit):
//   gcc teste_senha.c senha.c barra_tokens.c tokens_provisorio.c -o teste_senha.exe -lraylib -lopengl32 -lgdi32 -lwinmm
// Compilar (macOS):
//   cc teste_senha.c senha.c barra_tokens.c tokens_provisorio.c -o teste_senha $(pkg-config --cflags --libs raylib)
//
// Teclas: digite a senha, BACKSPACE apaga, ENTER confirma. No pop-up: clique ou 1 / 2.
// Depois de acertar: R reinicia.

#include "raylib.h"
#include "senha.h"
#include "tokens.h"
#include "barra_tokens.h"

int main(void)
{
    InitWindow(1280, 720, "re:7 - teste da senha (Ato 1)");
    SetTargetFPS(60);

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
            DrawText("Teste concluido. R reinicia.", 20, 690, 20, RAYWHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
