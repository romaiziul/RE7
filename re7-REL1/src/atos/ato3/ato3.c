#include "ato3.h"
#include "raylib.h"
#include "ui.h"

void Ato3_DesenharCenario(void)
{
    // Parede do auditorio.
    ClearBackground((Color){24, 23, 42, 255});

    DrawRectangleGradientV(
        170, 0, 940, 440,
        (Color){52, 48, 72, 255},
        (Color){27, 26, 45, 255}
    );

    // Luz sobre o palco.
    DrawTriangle(
        (Vector2){640, 35},
        (Vector2){310, 440},
        (Vector2){970, 440},
        Fade((Color){255, 226, 160, 255}, 0.13f)
    );

    // Cortinas.
    DrawRectangle(0, 0, 170, 465, (Color){102, 26, 51, 255});
    DrawRectangle(1110, 0, 170, 465, (Color){102, 26, 51, 255});

    for (int i = 0; i < 6; i++)
    {
        DrawRectangle(
            i * 30, 0, 12, 465,
            (Color){73, 20, 40, 255}
        );

        DrawRectangle(
            1110 + i * 30, 0, 12, 465,
            (Color){73, 20, 40, 255}
        );
    }

    DrawRectangle(0, 0, 1280, 65, (Color){82, 22, 43, 255});

    // Faixa do concurso.
    Rectangle faixa = {350, 85, 580, 65};

    DrawRectangleRounded(
        faixa, 0.15f, 8,
        (Color){232, 213, 169, 255}
    );

    const char *titulo = "CONCURSO DE ORATORIA";
    int largura = UiMeasureText(titulo, 28);

    UiDrawText(
        titulo, (1280 - largura) / 2, 106, 28,
        (Color){53, 40, 57, 255}
    );

    // Piso e borda do palco.
    DrawRectangle(170, 400, 940, 65, (Color){118, 77, 54, 255});
    DrawRectangle(170, 450, 940, 15, (Color){69, 44, 38, 255});

    // Lia, provisoriamente representada por formas.
    DrawCircle(640, 254, 28, (Color){230, 179, 146, 255});
    DrawRectangle(610, 282, 60, 85, (Color){141, 109, 181, 255});
    DrawCircle(640, 239, 29, (Color){55, 35, 39, 255});
    DrawCircle(640, 255, 23, (Color){230, 179, 146, 255});

    // Pulpito e computador.
    DrawRectangle(575, 340, 130, 95, (Color){101, 66, 45, 255});
    DrawRectangle(560, 327, 160, 18, (Color){155, 108, 65, 255});
    DrawRectangle(679, 288, 52, 38, (Color){17, 23, 32, 255});
    DrawRectangle(684, 293, 42, 28, (Color){59, 111, 124, 255});

    // Microfone.
    DrawLineEx(
        (Vector2){600, 328},
        (Vector2){615, 293},
        4, DARKGRAY
    );
    DrawCircle(615, 291, 6, BLACK);

    // Plateia.
    for (int i = 0; i < 12; i++)
    {
        int x = 90 + i * 100;

        DrawCircle(x, 484, 19, (Color){19, 20, 33, 255});
        DrawRectangleRounded(
            (Rectangle){x - 28, 501, 56, 55},
            0.3f, 8,
            (Color){30, 31, 49, 255}
        );
    }

    // Professora na primeira fileira.
    DrawCircle(190, 480, 19, (Color){201, 155, 124, 255});
    DrawRectangleRounded(
        (Rectangle){162, 499, 56, 55},
        0.3f, 8,
        (Color){83, 117, 107, 255}
    );

    // Amigo na plateia.
    DrawCircle(290, 480, 19, (Color){211, 165, 131, 255});
    DrawRectangleRounded(
        (Rectangle){262, 499, 56, 55},
        0.3f, 8,
        (Color){75, 107, 158, 255}
    );
}

void Ato3_DesenharPapel(void)
{
    Rectangle papel = {355, 175, 570, 315};

    // Sombra.
    DrawRectangleRounded(
        (Rectangle){363, 183, 570, 315},
        0.03f, 8, Fade(BLACK, 0.35f)
    );

    DrawRectangleRounded(
        papel, 0.03f, 8,
        (Color){244, 233, 204, 255}
    );

    Color tinta = {64, 54, 51, 255};

    UiDrawText("MEU DISCURSO", 390, 198, 24, tinta);
    DrawLine(390, 232, 890, 232, Fade(tinta, 0.3f));

    UiDrawText("Hoje e a primeira vez em muito tempo", 390, 255, 20, tinta);
    UiDrawText("que falo em", 390, 285, 20, tinta);
    UiDrawText("sabia? Entao estou bem nervosa,", 390, 315, 20, tinta);
    UiDrawText("mas pude contar com a ajuda de meus", 390, 345, 20, tinta);
    UiDrawText("para escrever esse texto,", 530, 375, 20, tinta);
    UiDrawText("entao me sinto bem mais", 390, 405, 20, tinta);

    // Manchas provisórias no lugar das palavras.
    Rectangle palavras[3] = {
        {515, 282, 125, 25},
        {390, 372, 125, 25},
        {665, 402, 165, 25}
    };

    for (int i = 0; i < 3; i++)
    {
        DrawRectangleRounded(
            palavras[i], 0.4f, 8,
            (Color){135, 126, 116, 255}
        );

        UiDrawText(
            "???",
            (int)palavras[i].x + 35,
            (int)palavras[i].y + 3,
            20, (Color){78, 72, 69, 255}
        );
    }

    UiDrawText(
        "As palavras parecem embaralhadas...",
        390, 452, 18, (Color){120, 88, 77, 255}
    );
}