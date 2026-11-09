#include "raylib.h"

typedef struct
{
    Rectangle area;
    const char *rotulo;
    const char *falaSeguinte;
} OpcaoDialogo;

int main(void)
{
    InitWindow(1280, 720, "Escolha de diálogo");
    SetTargetFPS(60);

    const Rectangle caixa = {120, 420, 1040, 270};
    const OpcaoDialogo opcoes[] = {
        {{144, 510, 992, 42}, "Tentar sozinha",
         "Lia respira fundo e volta a olhar para o papel."},
        {{144, 560, 992, 42}, "Pedir uma dica",
         "A IA elimina uma alternativa errada."},
        {{144, 610, 992, 42}, "Pedir que a IA leia",
         "A IA começa a ler o discurso em voz alta."}
    };

    const int totalOpcoes = sizeof(opcoes) / sizeof(opcoes[0]);
    const float duracao = 5.0f;
    float restante = duracao;
    int selecionada = -1;
    bool porTimeout = false;
    const char *fala = "Como você quer lidar com o discurso?";

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        if (selecionada == -1)
        {
            int escolhaDoQuadro = -1;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                for (int i = 0; i < totalOpcoes; i++)
                {
                    if (CheckCollisionPointRec(mouse, opcoes[i].area))
                    {
                        escolhaDoQuadro = i;
                        break;
                    }
                }
            }

            if (escolhaDoQuadro == -1)
            {
                restante -= GetFrameTime();
                if (restante <= 0.0f)
                {
                    restante = 0.0f;
                    escolhaDoQuadro = 0;
                    porTimeout = true;
                }
            }

            // Clique e timeout passam pelo mesmo ponto da história.
            if (escolhaDoQuadro != -1)
            {
                selecionada = escolhaDoQuadro;
                fala = opcoes[selecionada].falaSeguinte;
            }
        }

        BeginDrawing();
        ClearBackground((Color){25, 23, 45, 255});
        DrawRectangleRounded(caixa, 0.08f, 8, (Color){16, 19, 40, 255});
        DrawText(fala, 144, 446, 26, RAYWHITE);

        if (selecionada == -1)
        {
            DrawText(TextFormat("%.1f s", restante), 1048, 448, 22, SKYBLUE);
            DrawRectangle(144, 484, 992, 6, DARKGRAY);
            DrawRectangle(144, 484, (int)(992 * restante / duracao), 6, SKYBLUE);

            for (int i = 0; i < totalOpcoes; i++)
            {
                Rectangle area = opcoes[i].area;
                bool sobre = CheckCollisionPointRec(mouse, area);

                DrawRectangleRounded(area, 0.25f, 8,
                                     sobre ? (Color){108, 52, 115, 255} : DARKBLUE);
                DrawText(opcoes[i].rotulo, (int)area.x + 16, (int)area.y + 9,
                         24, RAYWHITE);
            }
        }
        else
        {
            DrawText(
                TextFormat("%s %s",
                           porTimeout ? "Tempo esgotado. Opção automática:" : "Você escolheu:",
                           opcoes[selecionada].rotulo),
                144, 500, 22, SKYBLUE
            );
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}