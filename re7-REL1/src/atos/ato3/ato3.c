#include "ato3.h"
#include "ato3_logica.h"
#include "raylib.h"
#include "ui.h"

#include <math.h>
#include <string.h>


// =====================================================
// AUDITORIO (desenhado em codigo, quadro 1280x720)
// =====================================================

#define COR_PELE_PLATEIA   (Color){19, 20, 33, 255}
#define COR_ROUPA_PLATEIA  (Color){30, 31, 49, 255}

// Uma pessoa da plateia. "pulo" sobe a cabeca durante os aplausos.
static void DesenharPessoa(int x, float pulo, Color pele, Color roupa)
{
    DrawCircle(x, (int)(484 - pulo), 19, pele);
    DrawRectangleRounded(
        (Rectangle){x - 28.0f, 501.0f - pulo * 0.4f, 56, 55},
        0.3f, 8, roupa
    );
}

// Duas maos batendo palmas acima da pessoa (tracinhos que abrem e fecham)
static void DesenharPalmas(int x, float fase)
{
    float abertura = 4.0f + 6.0f * fabsf(sinf(fase));
    Color cor = Fade((Color){255, 236, 190, 255}, 0.75f);

    DrawLineEx((Vector2){x - abertura, 470}, (Vector2){x - abertura - 5, 461}, 2.0f, cor);
    DrawLineEx((Vector2){x + abertura, 470}, (Vector2){x + abertura + 5, 461}, 2.0f, cor);
}

void Ato3_DesenharCenario(const Ato3Cena *cena)
{
    float t = (float)GetTime();

    // Parede do auditorio.
    ClearBackground((Color){24, 23, 42, 255});

    // Camera: na conclusao ela se aproxima da Lia, no pulpito.
    Camera2D camera = { 0 };
    camera.target = (Vector2){640, 330};
    camera.offset = (Vector2){640, 330};
    camera.zoom = 1.0f + 0.28f * cena->foco;

    BeginMode2D(camera);

    DrawRectangleGradientV(
        170, 0, 940, 440,
        (Color){52, 48, 72, 255},
        (Color){27, 26, 45, 255}
    );

    // Luz quente sobre o palco (mais forte quando a camera destaca a Lia).
    DrawTriangle(
        (Vector2){640, 35},
        (Vector2){310, 440},
        (Vector2){970, 440},
        Fade((Color){255, 226, 160, 255}, 0.13f + 0.12f * cena->foco)
    );

    // Cortinas.
    DrawRectangle(0, 0, 170, 465, (Color){102, 26, 51, 255});
    DrawRectangle(1110, 0, 170, 465, (Color){102, 26, 51, 255});

    for (int i = 0; i < 6; i++)
    {
        DrawRectangle(i * 30, 0, 12, 465, (Color){73, 20, 40, 255});
        DrawRectangle(1110 + i * 30, 0, 12, 465, (Color){73, 20, 40, 255});
    }

    DrawRectangle(0, 0, 1280, 65, (Color){82, 22, 43, 255});

    // Faixa do concurso.
    Rectangle faixa = {350, 85, 580, 65};

    DrawRectangleRounded(faixa, 0.15f, 8, (Color){232, 213, 169, 255});

    const char *titulo = "CONCURSO DE ORATÓRIA";
    int largura = UiMeasureText(titulo, 28);

    UiDrawText(titulo, (1280 - largura) / 2, 106, 28, (Color){53, 40, 57, 255});

    // Piso e borda do palco.
    DrawRectangle(170, 400, 940, 65, (Color){118, 77, 54, 255});
    DrawRectangle(170, 450, 940, 15, (Color){69, 44, 38, 255});

    // Lia (formas). Nervosa, ela treme um pouco.
    float tremor = sinf(t * 45.0f) * 2.0f * cena->nervosismo;
    int lx = 640 + (int)tremor;

    DrawCircle(lx, 254, 28, (Color){230, 179, 146, 255});
    DrawRectangle(lx - 30, 282, 60, 85, (Color){141, 109, 181, 255});
    DrawCircle(lx, 239, 29, (Color){55, 35, 39, 255});
    DrawCircle(lx, 255, 23, (Color){230, 179, 146, 255});

    // Pulpito e computador antigo.
    DrawRectangle(575, 340, 130, 95, (Color){101, 66, 45, 255});
    DrawRectangle(560, 327, 160, 18, (Color){155, 108, 65, 255});
    DrawRectangle(679, 288, 52, 38, (Color){17, 23, 32, 255});

    bool ligado = cena->pcOnline;
    if (cena->pcPiscando) ligado = fmodf(t * 7.0f, 1.0f) < 0.5f;

    Color tela = ligado ? (Color){74, 190, 135, 255} : (Color){25, 28, 35, 255};
    DrawRectangle(684, 293, 42, 28, tela);

    if (ligado)
    {
        // bonequinho do MSN na tela
        DrawCircle(700, 302, 4, (Color){230, 250, 240, 255});
        DrawCircle(710, 304, 4, (Color){120, 210, 255, 255});
        DrawRectangle(696, 307, 8, 6, (Color){230, 250, 240, 255});
        DrawRectangle(706, 309, 8, 6, (Color){120, 210, 255, 255});
    }

    // Microfone.
    DrawLineEx((Vector2){600, 328}, (Vector2){615, 293}, 4, DARKGRAY);
    DrawCircle(615, 291, 6, BLACK);

    // Plateia (a professora e o amigo na primeira fileira).
    for (int i = 0; i < 12; i++)
    {
        int x = 90 + i * 100;
        float fase = t * 11.0f + i * 1.7f;
        float pulo = cena->aplausos ? fabsf(sinf(fase * 0.5f)) * 4.0f : 0.0f;

        Color pele = COR_PELE_PLATEIA;
        Color roupa = COR_ROUPA_PLATEIA;

        if (i == 1) { pele = (Color){201, 155, 124, 255}; roupa = (Color){83, 117, 107, 255}; }  // professora
        if (i == 2) { pele = (Color){211, 165, 131, 255}; roupa = (Color){75, 107, 158, 255}; }  // amigo

        DesenharPessoa(x, pulo, pele, roupa);

        if (cena->aplausos) DesenharPalmas(x, fase);
    }

    // Rosto da professora: sorriso ou boca reta.
    Vector2 boca = {190, 488 - (cena->aplausos ? fabsf(sinf((t * 11.0f + 1.7f) * 0.5f)) * 4.0f : 0.0f)};
    Color corBoca = (Color){80, 45, 45, 255};

    if (cena->professoraSorri)
        DrawRing(boca, 6.0f, 8.0f, 20.0f, 160.0f, 12, corBoca);
    else
        DrawLineEx((Vector2){boca.x - 6, boca.y + 4}, (Vector2){boca.x + 6, boca.y + 4}, 2.0f, corBoca);

    EndMode2D();
}


// =====================================================
// PAPEL DO DISCURSO
// =====================================================

// O discurso, com as tres palavras marcadas: {0}, {1} e {2}.
// O que vem colado depois da marca ("," ou ".") e desenhado junto.
static const char *DISCURSO =
    "Hoje é a primeira vez em muito tempo que falo em {0}, sabia? "
    "Então estou bem nervosa, mas pude contar com a ajuda de meus {1} "
    "para escrever esse texto, então me sinto bem mais {2}.";

// Letras que substituem a palavra borrada (sempre as mesmas, sem piscar)
static void Embaralhar(const char *palavra, int indice, char *saida, int tamanhoSaida)
{
    static const char letras[] = "rnmwhkbdtqsv";
    int n = 0;

    for (int i = 0; palavra[i] != '\0' && n < tamanhoSaida - 1; i++)
    {
        // conta so o primeiro byte de cada letra (UTF-8)
        if (((unsigned char)palavra[i] & 0xC0) == 0x80) continue;

        saida[n] = letras[(i * 7 + indice * 5 + 3) % (int)(sizeof(letras) - 1)];
        n++;
    }

    saida[n] = '\0';
}

static void DesenharPalavraBorrada(
    const char *palavra, int indice, float nitidez, bool destaque,
    Vector2 pos, int fs, float largura, Color tinta)
{
    float t = (float)GetTime();
    float borrado = 1.0f - nitidez;

    if (borrado > 0.01f)
    {
        char falso[32];
        Embaralhar(palavra, indice, falso, sizeof(falso));

        // mancha de tinta por baixo
        DrawRectangleRounded(
            (Rectangle){pos.x - 4, pos.y + 2, largura + 8, fs + 4.0f},
            0.5f, 8, Fade(tinta, 0.12f * borrado)
        );

        // a mesma palavra falsa repetida em volta: parece desfocada
        float raio = 0.6f + 3.4f * borrado;

        for (int k = 0; k < 8; k++)
        {
            float ang = k * (PI / 4.0f) + t * 0.6f;
            UiDrawText(
                falso,
                (int)(pos.x + cosf(ang) * raio),
                (int)(pos.y + sinf(ang) * raio),
                fs, Fade(tinta, 0.17f * borrado)
            );
        }
    }

    if (nitidez > 0.0f)
        UiDrawTextEmphasis(palavra, (int)pos.x, (int)pos.y, fs, Fade(tinta, nitidez));

    // palavra da etapa atual: sublinhado que pulsa
    if (destaque && nitidez < 1.0f)
    {
        float pulso = 0.55f + 0.45f * sinf(t * 4.0f);
        DrawRectangle(
            (int)pos.x, (int)(pos.y + fs + 6), (int)largura, 3,
            Fade((Color){255, 105, 200, 255}, pulso)
        );
    }
}

void Ato3_DesenharPapel(
    Rectangle area,
    int tamanhoFonte,
    const float nitidez[3],
    int destaque,
    Vector2 tremor)
{
    area.x += tremor.x;
    area.y += tremor.y;

    Color tinta = {64, 54, 51, 255};
    int fs = tamanhoFonte;
    int altLinha = (int)(fs * 1.45f);
    int margem = (int)(fs * 1.4f);

    // Sombra e folha.
    DrawRectangleRounded(
        (Rectangle){area.x + 8, area.y + 8, area.width, area.height},
        0.03f, 8, Fade(BLACK, 0.35f)
    );
    DrawRectangleRounded(area, 0.03f, 8, (Color){244, 233, 204, 255});

    int tituloFs = fs + 2;
    UiDrawTextEmphasis("MEU DISCURSO", (int)area.x + margem, (int)area.y + (int)(fs * 0.8f), tituloFs, tinta);

    float linhaTitulo = area.y + fs * 0.8f + tituloFs + 8;
    DrawLine((int)area.x + margem, (int)linhaTitulo, (int)(area.x + area.width) - margem, (int)linhaTitulo, Fade(tinta, 0.3f));

    // Linhas de caderno, bem claras, logo abaixo de cada linha de texto.
    for (float ly = linhaTitulo + fs * 0.7f + fs + 7; ly < area.y + area.height - 10; ly += altLinha)
        DrawLine((int)area.x + 12, (int)ly, (int)(area.x + area.width) - 12, (int)ly, Fade((Color){120, 160, 200, 255}, 0.18f));

    // Texto com quebra de linha, palavra por palavra.
    float esquerda = area.x + margem;
    float direita = area.x + area.width - margem;
    float espaco = (float)(UiMeasureText("a a", fs) - UiMeasureText("aa", fs));
    float x = esquerda;
    float y = linhaTitulo + fs * 0.7f;
    bool inicioDaLinha = true;

    const char *p = DISCURSO;

    while (*p != '\0')
    {
        char palavra[64];
        int n = 0;

        while (*p != '\0' && *p != ' ' && n < (int)sizeof(palavra) - 1) palavra[n++] = *p++;
        palavra[n] = '\0';
        while (*p == ' ') p++;

        if (n == 0) continue;

        int indice = -1;
        const char *sufixo = "";
        const char *texto = palavra;

        if (palavra[0] == '{' && palavra[1] >= '0' && palavra[1] <= '2' && palavra[2] == '}')
        {
            indice = palavra[1] - '0';
            sufixo = &palavra[3];
            texto = Ato3_Etapa(ATO3_AJUDA_SOZINHA, indice)->palavra;
        }

        float largPalavra = (indice >= 0) ? (float)UiMeasureTextEmphasis(texto, fs) : (float)UiMeasureText(texto, fs);
        float largSufixo = (float)UiMeasureText(sufixo, fs);
        float largTotal = largPalavra + largSufixo;

        if (!inicioDaLinha && x + espaco + largTotal > direita)
        {
            x = esquerda;
            y += altLinha;
            inicioDaLinha = true;
        }

        if (!inicioDaLinha) x += espaco;

        if (indice >= 0)
        {
            DesenharPalavraBorrada(texto, indice, nitidez[indice], indice == destaque,
                                   (Vector2){x, y}, fs, largPalavra, tinta);
            UiDrawText(sufixo, (int)(x + largPalavra), (int)y, fs, tinta);
        }
        else
        {
            UiDrawText(texto, (int)x, (int)y, fs, tinta);
        }

        x += largTotal;
        inicioDaLinha = false;
    }
}
