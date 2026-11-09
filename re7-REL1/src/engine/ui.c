#include "ui.h"

#include <math.h>
#include <stdio.h>

static Font fonteRegular;
static Font fonteSemibold;

static Font CarregarFonteUi(const char *caminho, const int *caracteres, int total)
{
    Font fonte = LoadFontEx(caminho, 64, caracteres, total);
    if (fonte.texture.id == 0 || fonte.texture.id == GetFontDefault().texture.id)
        return (Font){0};

    SetTextureFilter(fonte.texture, TEXTURE_FILTER_BILINEAR);
    return fonte;
}

void UiLoadFonts(void)
{
    // ASCII, Latin-1 (acentos e simbolos da senha) e pontuacao tipografica.
    int caracteres[200];
    int total = 0;

    for (int letra = 32; letra <= 126; letra++) caracteres[total++] = letra;
    for (int letra = 160; letra <= 255; letra++) caracteres[total++] = letra;

    const int pontuacao[] = {0x2013, 0x2014, 0x2018, 0x2019, 0x201C, 0x201D, 0x2026};
    for (int i = 0; i < (int)(sizeof(pontuacao) / sizeof(pontuacao[0])); i++)
        caracteres[total++] = pontuacao[i];

    fonteRegular = CarregarFonteUi("assets/fonts/Nunito-Regular.ttf", caracteres, total);
    fonteSemibold = CarregarFonteUi("assets/fonts/Nunito-Semibold.ttf", caracteres, total);
}

void UiUnloadFonts(void)
{
    if (fonteRegular.texture.id != 0) UnloadFont(fonteRegular);
    if (fonteSemibold.texture.id != 0) UnloadFont(fonteSemibold);

    fonteRegular = (Font){0};
    fonteSemibold = (Font){0};
}

static float EspacoEntreLetras(int tamanho)
{
    return tamanho * 0.025f;
}

static void DesenharComFonte(Font fonte, const char *text, int x, int y,
                             int fontSize, Color color)
{
    if (fonte.texture.id == 0)
    {
        DrawText(text, x, y, fontSize, color);
        return;
    }

    DrawTextEx(fonte, text, (Vector2){(float)x, (float)y},
               (float)fontSize, EspacoEntreLetras(fontSize), color);
}

static int MedirComFonte(Font fonte, const char *text, int fontSize)
{
    if (fonte.texture.id == 0) return MeasureText(text, fontSize);

    Vector2 tamanho = MeasureTextEx(fonte, text, (float)fontSize,
                                   EspacoEntreLetras(fontSize));
    return (int)ceilf(tamanho.x);
}

void UiDrawText(const char *text, int x, int y, int fontSize, Color color)
{
    DesenharComFonte(fonteRegular, text, x, y, fontSize, color);
}

int UiMeasureText(const char *text, int fontSize)
{
    return MedirComFonte(fonteRegular, text, fontSize);
}

void UiDrawTextEmphasis(const char *text, int x, int y, int fontSize, Color color)
{
    if (fonteSemibold.texture.id == 0)
        UiDrawText(text, x, y, fontSize, color);
    else
        DesenharComFonte(fonteSemibold, text, x, y, fontSize, color);
}

int UiMeasureTextEmphasis(const char *text, int fontSize)
{
    if (fonteSemibold.texture.id == 0) return UiMeasureText(text, fontSize);

    return MedirComFonte(fonteSemibold, text, fontSize);
}


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
        UiMeasureText(
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


    UiDrawText(
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

        if (UiMeasureText(teste, fontSize) > larguraMax && linha[0] != '\0')
        {
            UiDrawText(linha, x, y + linhas * lineHeight, fontSize, color);
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
        UiDrawText(linha, x, y + linhas * lineHeight, fontSize, color);
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


    UiDrawTextEmphasis(
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
        576,
        28,
        32,
        880,
        WHITE
    );


    UiDrawText(
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


    UiDrawText(

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