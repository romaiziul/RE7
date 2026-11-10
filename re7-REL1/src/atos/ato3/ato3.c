#include "ato3.h"
#include "ato3_logica.h"
#include "raylib.h"
#include "ui.h"

#include <math.h>
#include <string.h>


// =====================================================
// AUDITORIO (assets pintados, quadro 1280x720)
// =====================================================

// Recorte do fundo: o palco fica centralizado no quadro (o lado direito da
// imagem e so parede). 1320 x 742.5 e 16:9, como a janela.
#define AUD_RECORTE_LARGURA  1320.0f
#define AUD_RECORTE_ALTURA    742.5f

// Cores tiradas dos assets pintados
#define COR_TINTA_FAIXA   (Color){ 92,  58,  52, 255}   // letras na faixa de pano
#define COR_LUZ_QUENTE    (Color){255, 214, 150, 255}   // refletor e brilhos dos aplausos
#define COR_MADEIRA_CLARA (Color){178, 112,  70, 255}   // tampo do pulpito (piso do palco)
#define COR_MADEIRA       (Color){134,  80,  52, 255}
#define COR_MADEIRA_ESC   (Color){ 94,  54,  40, 255}
#define COR_CONTORNO      (Color){ 82,  46,  36, 255}   // contorno fino, como nos sprites

// Lia e pulpito (centro do palco no recorte)
#define LIA_ALTURA     400.0f
#define LIA_TOPO       175.0f
#define PULPITO_TOPO   420.0f

static void DesenharFundo(Texture2D fundo)
{
    if (fundo.id == 0)
    {
        ClearBackground((Color){52, 36, 48, 255});
        return;
    }

    DrawTexturePro(
        fundo,
        (Rectangle){0, 0, AUD_RECORTE_LARGURA, AUD_RECORTE_ALTURA},
        (Rectangle){0, 0, 1280, 720},
        (Vector2){0, 0}, 0.0f, WHITE
    );
}

// Titulo pintado na faixa de pano em branco do fundo
static void DesenharTituloFaixa(void)
{
    const char *titulo = "CONCURSO DE ORATÓRIA";
    int fs = 34;
    int x = (1280 - UiMeasureTextEmphasis(titulo, fs)) / 2;

    UiDrawTextEmphasis(titulo, x, 136, fs, Fade(COR_TINTA_FAIXA, 0.88f));
}

static void DesenharPulpito(void)
{
    // corpo (a parte de baixo fica atras da caixa de dialogo)
    Rectangle corpo = {552, PULPITO_TOPO + 16, 176, 150};
    DrawRectangleGradientV((int)corpo.x, (int)corpo.y, (int)corpo.width, (int)corpo.height,
                           COR_MADEIRA, COR_MADEIRA_ESC);
    DrawRectangleLinesEx(corpo, 2.0f, COR_CONTORNO);

    // painel frontal em baixo-relevo
    Rectangle painel = {corpo.x + 18, corpo.y + 18, corpo.width - 36, corpo.height - 36};
    DrawRectangleLinesEx(painel, 1.5f, Fade(COR_CONTORNO, 0.6f));
    DrawLineEx((Vector2){painel.x + 2, painel.y + 2}, (Vector2){painel.x + painel.width - 2, painel.y + 2},
               1.5f, Fade(COR_MADEIRA_CLARA, 0.5f));

    // tampo inclinado
    Rectangle tampo = {536, PULPITO_TOPO, 208, 20};
    DrawRectangleRounded(tampo, 0.4f, 8, COR_MADEIRA_CLARA);
    DrawRectangleRoundedLines(tampo, 0.4f, 8, COR_CONTORNO);
    DrawLineEx((Vector2){tampo.x + 10, tampo.y + 5}, (Vector2){tampo.x + tampo.width - 10, tampo.y + 5},
               2.0f, Fade(WHITE, 0.18f));

    // microfone
    // (abaixo do queixo da Lia, para nao cobrir o rosto)
    DrawLineEx((Vector2){570, PULPITO_TOPO + 2}, (Vector2){584, PULPITO_TOPO - 26}, 3.0f, COR_CONTORNO);
    DrawCircleV((Vector2){585, PULPITO_TOPO - 30}, 6.5f, (Color){58, 44, 50, 255});
    DrawCircleV((Vector2){583, PULPITO_TOPO - 32}, 2.0f, Fade(WHITE, 0.35f));
}

// Monitor antigo no canto do pulpito. A tela acende com o MSN.
static void DesenharMonitor(Texture2D monitor, bool ligado)
{
    const float lado = 92.0f;
    Rectangle destino = {700, PULPITO_TOPO - lado + 6, lado, lado};

    if (monitor.id != 0)
    {
        DrawTexturePro(monitor, (Rectangle){0, 0, (float)monitor.width, (float)monitor.height},
                       destino, (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleRounded(destino, 0.15f, 8, (Color){226, 205, 180, 255});
    }

    if (!ligado) return;

    // area da tela dentro da imagem do monitor (proporcoes do asset)
    Rectangle tela = {
        destino.x + destino.width * 0.125f,
        destino.y + destino.height * 0.16f,
        destino.width * 0.62f,
        destino.height * 0.53f
    };

    DrawRectangleRounded(tela, 0.12f, 6, (Color){92, 186, 140, 235});
    DrawRectangleGradientV((int)tela.x + 2, (int)tela.y + 2, (int)tela.width - 4, (int)(tela.height * 0.45f),
                           Fade(WHITE, 0.22f), Fade(WHITE, 0.0f));

    // as duas figurinhas do MSN
    float cx = tela.x + tela.width / 2;
    float cy = tela.y + tela.height / 2;
    DrawCircleV((Vector2){cx - 6, cy - 5}, 4.0f, (Color){238, 250, 242, 255});
    DrawCircleV((Vector2){cx + 6, cy - 3}, 4.0f, (Color){150, 214, 250, 255});
    DrawRectangleRounded((Rectangle){cx - 10, cy, 8, 7}, 0.5f, 4, (Color){238, 250, 242, 255});
    DrawRectangleRounded((Rectangle){cx + 2, cy + 2, 8, 7}, 0.5f, 4, (Color){150, 214, 250, 255});

    // brilho verde da tela no rosto da Lia e no tampo
    DrawCircleGradient((Vector2){cx, cy}, 70.0f, Fade((Color){120, 220, 170, 255}, 0.10f), Fade(BLANK, 0.0f));
}

// Aplausos: a luz do palco esquenta e brilhos dourados sobem da plateia
static void DesenharAplausos(float t)
{
    for (int i = 0; i < 46; i++)
    {
        float x = fmodf(i * 211.7f + 40.0f, 1280.0f) + sinf(t * 1.3f + i) * 10.0f;
        float subida = fmodf(t * (38.0f + (i % 5) * 9.0f) + i * 61.0f, 560.0f);
        float y = 720.0f - subida;
        float vida = 1.0f - subida / 560.0f;
        float r = 1.6f + (i % 3) * 1.1f;

        DrawCircleV((Vector2){x, y}, r * 3.2f, Fade(COR_LUZ_QUENTE, 0.07f * vida));
        DrawCircleV((Vector2){x, y}, r, Fade(COR_LUZ_QUENTE, 0.75f * vida));
    }
}

void Ato3_DesenharCenario(const Ato3Cena *cena, const Assets *assets)
{
    float t = (float)GetTime();

    ClearBackground(BLACK);

    // Camera: na conclusao ela se aproxima da Lia, no pulpito.
    Camera2D camera = { 0 };
    camera.target = (Vector2){640, 330};
    camera.offset = (Vector2){640, 330};
    camera.zoom = 1.0f + 0.28f * cena->foco;

    BeginMode2D(camera);

    DesenharFundo(assets->bgAuditorio);
    DesenharTituloFaixa();

    // Refletor sobre a Lia (mais forte quando a camera a destaca ou ha aplausos)
    float luz = 0.10f + 0.10f * cena->foco + (cena->aplausos ? 0.06f + 0.03f * sinf(t * 2.0f) : 0.0f);
    DrawCircleGradient((Vector2){640, 330}, 330.0f, Fade(COR_LUZ_QUENTE, luz), Fade(COR_LUZ_QUENTE, 0.0f));

    // Lia atras do pulpito. Nervosa, ela treme um pouco.
    if (cena->lia.id != 0)
    {
        float tremor = sinf(t * 45.0f) * 2.0f * cena->nervosismo;
        float largura = LIA_ALTURA * (float)cena->lia.width / (float)cena->lia.height;

        DrawTexturePro(
            cena->lia,
            (Rectangle){0, 0, (float)cena->lia.width, (float)cena->lia.height},
            (Rectangle){640 - largura / 2 + tremor, LIA_TOPO, largura, LIA_ALTURA},
            (Vector2){0, 0}, 0.0f, WHITE
        );
    }

    DesenharPulpito();

    bool ligado = cena->pcOnline;
    if (cena->pcPiscando) ligado = fmodf(t * 7.0f, 1.0f) < 0.5f;

    DesenharMonitor(assets->monitorCrt, ligado);

    EndMode2D();

    if (cena->aplausos) DesenharAplausos(t);
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
    Vector2 tremor,
    Texture2D folha)
{
    area.x += tremor.x;
    area.y += tremor.y;

    Color tinta = {64, 54, 51, 255};
    int fs = tamanhoFonte;
    int altLinha = (int)(fs * 1.45f);
    int margem = (int)(fs * 1.4f);

    // Sombra e folha.
    if (folha.id != 0)
    {
        // a imagem tem uma borda transparente: amplia para a folha cobrir a area
        const float bordaX = 0.037f, bordaCima = 0.021f, bordaBaixo = 0.028f;
        float largura = area.width / (1.0f - 2.0f * bordaX);
        float altura = area.height / (1.0f - bordaCima - bordaBaixo);
        Rectangle origem = {0, 0, (float)folha.width, (float)folha.height};
        Rectangle destino = {area.x - largura * bordaX, area.y - altura * bordaCima, largura, altura};

        // sombra com o mesmo recorte da folha (inclui a ponta dobrada)
        DrawTexturePro(folha, origem, (Rectangle){destino.x + 7, destino.y + 7, largura, altura},
                       (Vector2){0, 0}, 0.0f, Fade(BLACK, 0.32f));
        DrawTexturePro(folha, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleRounded(
            (Rectangle){area.x + 8, area.y + 8, area.width, area.height},
            0.03f, 8, Fade(BLACK, 0.30f)
        );
        DrawRectangleRounded(area, 0.03f, 8, (Color){244, 233, 204, 255});
    }

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
