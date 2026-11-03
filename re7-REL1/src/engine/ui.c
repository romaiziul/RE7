#include "ui.h"

#include <stdio.h>


// =====================================================
// TEXTO CENTRALIZADO
// =====================================================

void DrawCenteredText(
    const char *text,
    int y,
    int fontSize,
    Color color
)
{
    int width =
        MeasureText(
            text,
            fontSize
        );


    int x =
        (
            GetScreenWidth()
            -
            width
        )
        /
        2;


    DrawText(
        text,
        x,
        y,
        fontSize,
        color
    );
}


// =====================================================
// BOTÃO
// =====================================================

void DrawImageButton(
    Texture2D normal,
    Texture2D hover,
    Texture2D pressed,
    Rectangle rect
)
{
    Vector2 mouse =
        GetMousePosition();


    bool mouseOver =
        CheckCollisionPointRec(
            mouse,
            rect
        );


    Texture2D texture =
        normal;


    if (mouseOver)
    {
        texture =
            hover;
    }


    if (
        mouseOver
        &&
        IsMouseButtonDown(
            MOUSE_BUTTON_LEFT
        )
    )
    {
        texture =
            pressed;
    }


    DrawTexturePro(

        texture,

        (Rectangle)
        {
            0,
            0,
            texture.width,
            texture.height
        },

        rect,

        (Vector2)
        {
            0,
            0
        },

        0,

        WHITE
    );
}


// =====================================================
// DETECTAR CLIQUE
// =====================================================

bool IsButtonClicked(
    Rectangle rect
)
{
    Vector2 mouse =
        GetMousePosition();


    return
        CheckCollisionPointRec(
            mouse,
            rect
        )

        &&

        IsMouseButtonPressed(
            MOUSE_BUTTON_LEFT
        );
}


// =====================================================
// CAIXA DE DIÁLOGO
// =====================================================

// Escreve um texto quebrando em linhas dentro de "larguraMax".
// Retorna quantas linhas foram usadas.
static int DrawWrappedText(
    const char *text,
    int x,
    int y,
    int fontSize,
    int lineHeight,
    int larguraMax,
    Color color
)
{
    char linha[1000] = "";
    int linhas = 0;
    int pos = 0;

    while (text[pos] != '\0')
    {
        // pega a proxima palavra
        char palavra[128] = "";
        int n = 0;

        while (text[pos] != '\0' && text[pos] != ' ' && n < 127)
        {
            palavra[n++] = text[pos++];
        }
        palavra[n] = '\0';

        while (text[pos] == ' ') pos++;

        // testa se a palavra cabe na linha atual
        char teste[1200];

        if (linha[0] == '\0')
            snprintf(teste, sizeof(teste), "%s", palavra);
        else
            snprintf(teste, sizeof(teste), "%s %s", linha, palavra);

        if (MeasureText(teste, fontSize) > larguraMax && linha[0] != '\0')
        {
            DrawText(linha, x, y + linhas * lineHeight, fontSize, color);
            linhas++;
            snprintf(linha, sizeof(linha), "%s", palavra);
        }
        else
        {
            snprintf(linha, sizeof(linha), "%s", teste);
        }
    }

    if (linha[0] != '\0')
    {
        DrawText(linha, x, y + linhas * lineHeight, fontSize, color);
        linhas++;
    }

    return linhas;
}


void DrawDialogueBox(
    const char *speaker,
    const char *text
)
{
    // fala vazia: nao desenha a caixa (usado nas imagens de cena)
    if (text[0] == '\0' && speaker[0] == '\0') return;

    Rectangle box =
    {
        120,
        520,
        1040,
        160
    };


    DrawRectangleRounded(
        box,
        0.08f,
        10,
        Fade(BLACK, 0.80f)
    );


    DrawText(
        speaker,
        160,
        538,
        26,
        PINK
    );


    // texto com quebra de linha (ate 3 linhas)
    DrawWrappedText(
        text,
        160,
        578,
        26,
        32,
        880,
        WHITE
    );


    DrawText(
        ">>",
        1090,
        640,
        28,
        SKYBLUE
    );
}


// =====================================================
// DEBUG GRID
// =====================================================

void DrawDebugGrid(void)
{
    for (
        int x = 0;
        x < GetScreenWidth();
        x += 100
    )
    {
        DrawLine(
            x,
            0,
            x,
            GetScreenHeight(),
            Fade(WHITE, 0.25f)
        );
    }


    for (
        int y = 0;
        y < GetScreenHeight();
        y += 100
    )
    {
        DrawLine(
            0,
            y,
            GetScreenWidth(),
            y,
            Fade(WHITE, 0.25f)
        );
    }
}


// =====================================================
// POSIÇÃO DO MOUSE
// =====================================================

void DrawMousePosition(void)
{
    Vector2 mouse =
        GetMousePosition();


    DrawText(

        TextFormat(
            "X: %.0f Y: %.0f",
            mouse.x,
            mouse.y
        ),

        20,
        20,
        20,
        YELLOW
    );
}