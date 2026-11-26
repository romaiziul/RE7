#include "ato3.h"
#include "ato3_logica.h"
#include "raylib.h"
#include "ui.h"

#include <math.h>
#include <string.h>


// Arte do Ato 3 em camadas; regras e transicoes continuam no nucleo.
static void DesenharArte(Texture2D textura, Rectangle destino)
{
    if (!textura.id || textura.width <= 0 || textura.height <= 0) return;
    DrawTexturePro(textura,
        (Rectangle){0, 0, (float)textura.width, (float)textura.height},
        destino, (Vector2){0, 0}, 0, WHITE);
}

// A altura e a ancora inferior sao comuns as duas expressoes.
static void DesenharLiaPalco(Texture2D textura, float centroX, float baseY, float altura)
{
    if (!textura.id || textura.height <= 0) return;
    float largura = altura * textura.width / textura.height;
    float topo = baseY - altura;
    // O retrato termina atras da borda do pulpito, sem mostrar um torso
    // cortado ao lado da madeira. Recorte no mundo, compativel com o zoom.
    float visivel = fminf(altura, 340.0f - topo);
    if (visivel <= 0) return;
    DrawTexturePro(textura,
        (Rectangle){0, 0, (float)textura.width, textura.height * visivel / altura},
        (Rectangle){centroX - largura / 2, topo, largura, visivel},
        (Vector2){0, 0}, 0, WHITE);
}

void Ato3_DesenharCenario(const Ato3Cena *cena, const Assets *assets)
{
    float t = (float)GetTime();
    ClearBackground((Color){39, 30, 45, 255});
    Camera2D camera = {0};
    camera.target = (Vector2){680, 330};
    camera.offset = camera.target;
    camera.zoom = 1.0f + 0.28f * cena->foco;
    BeginMode2D(camera);

    DesenharArte(assets->ato3Auditorio, (Rectangle){0, 0, 1280, 720});
    const char *titulo = "CONCURSO DE ORATÓRIA";
    UiDrawText(titulo, 500 - UiMeasureText(titulo, 31) / 2, 112, 31,
               (Color){83, 53, 49, 255});

    // Aplausos nao significam que Lia sorri: no final em que a IA le,
    // professoraSorri permanece false, preservando a expressao tensa.
    Texture2D lia = cena->professoraSorri ? assets->lia07ConfiantePalco : assets->lia07NervosaPalco;
    float tremor = sinf(t * 45.0f) * 2.0f * cena->nervosismo;
    DesenharLiaPalco(lia, 680 + tremor, 384, 238);
    DesenharArte(assets->ato3Pulpito, (Rectangle){580, 250, 205, 307.5f});

    bool ligado = cena->pcOnline;
    if (cena->pcPiscando) ligado = fmodf(t * 7.0f, 1.0f) < 0.5f;
    DesenharArte(ligado ? assets->ato3PcLigado : assets->ato3PcDesligado,
                 (Rectangle){705, 283, 65, 65});

    // O texto de status fica na interface; as imagens nao contem letras.
    // Movimento sutil acompanha a pose de aplauso, sem piscar os rostos.
    float pulo = cena->aplausos ? fabsf(sinf(t * 5.5f)) * 2.0f : 0.0f;
    Texture2D plateia = cena->aplausos ? assets->ato3PlateiaAplausos : assets->ato3PlateiaAtenta;
    if (plateia.id && plateia.width > 0)
        DesenharArte(plateia, (Rectangle){0, 445 - pulo, 1280,
                                         1280.0f * plateia.height / plateia.width});
    EndMode2D();
}

// PAPEL DO DISCURSO: a textura e somente a folha; palavras e efeitos
// continuam desenhados separadamente para responder ao minijogo.
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
    const Assets *assets,
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

    // A sombra usa o mesmo recorte da folha, sem um retangulo nas margens transparentes.
    if (assets->ato3Papel.id)
    {
        Texture2D folha = assets->ato3Papel;
        DrawTexturePro(folha, (Rectangle){0, 0, (float)folha.width, (float)folha.height},
                       (Rectangle){area.x + 8, area.y + 8, area.width, area.height},
                       (Vector2){0, 0}, 0, Fade(BLACK, 0.35f));
        DesenharArte(assets->ato3Papel, area);
    }
    else
        DrawRectangleRounded(area, 0.03f, 8, (Color){244, 233, 204, 255});

    int tituloFs = fs + 2;
    UiDrawTextEmphasis("MEU DISCURSO", (int)area.x + margem, (int)area.y + (int)(fs * 0.8f), tituloFs, tinta);

    float linhaTitulo = area.y + fs * 0.8f + tituloFs + 8;
    DrawLine((int)area.x + margem, (int)linhaTitulo, (int)(area.x + area.width) - margem, (int)linhaTitulo, Fade(tinta, 0.3f));

    // Linhas de caderno, bem claras, logo abaixo de cada linha de texto.
    for (float ly = linhaTitulo + fs * 0.7f + fs + 7; ly < area.y + area.height - 10; ly += altLinha)
        DrawLine((int)area.x + margem, (int)ly, (int)(area.x + area.width) - margem, (int)ly, Fade((Color){120, 160, 200, 255}, 0.18f));

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
