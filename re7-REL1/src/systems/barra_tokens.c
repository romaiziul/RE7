// barra_tokens.c
// Barra de tokens da IA, sem caixa de fundo: avatar redondo rosa com o simbolo
// da IA (assets/comum/iaStar.png, o mesmo da caixa de falas), status do MSN,
// UMA bateria dividida em 3 partes, como a de um celular, e textos claros com
// contorno escuro fino, para ler tanto em cenas escuras quanto claras.
//
// Cores da bateria:
// - tokens normais: degrade do rosa (em cima) para o azul (embaixo);
// - token de reserva (Ato 3): vermelho;
// - partes vazias e barra em 0: cinza (com 0, o status vira "offline").
//
// Animacoes:
// - ao perder um token, a parte vai do azul para o cinza;
// - ao ganhar um token (sozinha ou reserva), a parte acende com brilho rosa.

#include "barra_tokens.h"
#include "tokens.h"
#include "raylib.h"
#include "ui.h"
#include <math.h>

#define COR_CLARA    (Color){ 232, 240, 250, 255 }  // #E8F0FA
#define COR_AZUL_MED (Color){ 143, 178, 213, 255 }  // #8FB2D5
#define COR_MARINHO  (Color){  29,  38,  96, 255 }  // #1D2660
#define COR_ROSA     (Color){ 227,  93, 215, 255 }  // #E35DD7
#define COR_AZUL     (Color){  39, 172, 234, 255 }  // #27ACEA
#define COR_VERMELHO (Color){ 234,  84, 119, 255 }  // #EA5477
#define COR_VERDE    (Color){  76, 175,  80, 255 }
#define COR_CINZA    (Color){ 150, 156, 170, 255 }

#define COR_ROSA_CLARO (Color){ 250, 233, 247, 255 }  // fundo da bateria
#define COR_AVATAR_CENTRO (Color){ 255, 175, 235, 255 }  // fundo do avatar: rosa claro no centro
#define COR_AVATAR_BORDA  (Color){ 240, 125, 222, 255 }  // e um pouco mais forte na borda

#define DURACAO_ANIM 0.6f

// Simbolo da IA. Carregado no primeiro desenho (precisa da janela aberta).
static Texture2D simboloIA;
static bool simboloCarregado = false;

static void CarregarSimbolo(void)
{
    if (simboloCarregado) return;
    simboloCarregado = true;
    simboloIA = LoadTexture("assets/comum/iaStar.png");
    if (simboloIA.id != 0) {
        // a imagem tem 256 px e aparece com 38 px: os mipmaps evitam que o anel fique serrilhado
        GenTextureMipmaps(&simboloIA);
        SetTextureFilter(simboloIA, TEXTURE_FILTER_TRILINEAR);
    }
}

void BarraTokens_Descarregar(void)
{
    if (simboloCarregado && simboloIA.id != 0) UnloadTexture(simboloIA);
    simboloIA = (Texture2D){0};
    simboloCarregado = false;
}

static int   valorAnterior = -1;
static float animPerda[TOKENS_MAX];   // tempo restante da animacao de perda
static float animGanho[TOKENS_MAX];   // tempo restante da animacao de ganho
static float tempoTotal = 0.0f;       // para o brilho "respirando"

void BarraTokens_Reiniciar(void)
{
    for (int i = 0; i < TOKENS_MAX; i++) {
        animPerda[i] = 0.0f;
        animGanho[i] = 0.0f;
    }
    valorAnterior = Tokens_Valor();
}

void BarraTokens_Atualizar(void)
{
    float dt = GetFrameTime();
    tempoTotal += dt;

    int atual = Tokens_Valor();
    if (valorAnterior < 0) valorAnterior = atual;

    if (atual < valorAnterior) {
        for (int i = atual; i < valorAnterior; i++) animPerda[i] = DURACAO_ANIM;
    } else if (atual > valorAnterior) {
        for (int i = valorAnterior; i < atual; i++) animGanho[i] = DURACAO_ANIM;
    }
    valorAnterior = atual;

    for (int i = 0; i < TOKENS_MAX; i++) {
        if (animPerda[i] > 0.0f) animPerda[i] -= dt;
        if (animGanho[i] > 0.0f) animGanho[i] -= dt;
    }
}

// Mistura duas cores (t = 0 da a primeira, t = 1 da a segunda)
static Color Misturar(Color a, Color b, float t)
{
    return (Color){ (unsigned char)(a.r + (b.r - a.r) * t),
                    (unsigned char)(a.g + (b.g - a.g) * t),
                    (unsigned char)(a.b + (b.b - a.b) * t), 255 };
}

// Reflexo branco na metade de cima, que da o efeito de vidro
static void Reflexo(Rectangle r, float opacidade)
{
    Rectangle topo = { r.x + 3, r.y + 2, r.width - 6, r.height * 0.42f };
    DrawRectangleRounded(topo, 1.0f, 8, Fade(WHITE, opacidade));
}

static void DesenharBateria(Rectangle corpo, int valor, bool offline)
{
    Color contorno = offline ? COR_CINZA : COR_CLARA;

    // corpo e polo da bateria
    // fundo opaco da bateria
    DrawRectangleRounded(corpo, 0.35f, 8, offline ? COR_CLARA : COR_ROSA_CLARO);
    DrawRectangleRounded(corpo, 0.35f, 8, Fade(COR_MARINHO, 0.10f));
    DrawRectangleRoundedLinesEx(corpo, 0.35f, 8, 2.5f, contorno);
    Rectangle polo = { corpo.x + corpo.width + 2, corpo.y + corpo.height * 0.3f, 6, corpo.height * 0.4f };
    // contorno escuro fino por fora, para a bateria nao sumir em fundo claro
    Color bordaEscura = Fade((Color){ 20, 22, 60, 255 }, 0.85f);
    DrawRectangleRoundedLinesEx((Rectangle){ corpo.x - 1.5f, corpo.y - 1.5f, corpo.width + 3, corpo.height + 3 },
                                0.35f, 8, 1.2f, bordaEscura);
    DrawRectangleRounded((Rectangle){ polo.x - 1, polo.y - 1, polo.width + 2, polo.height + 2 }, 0.5f, 4, bordaEscura);
    DrawRectangleRounded(polo, 0.5f, 4, contorno);

    float pulso = 0.85f + 0.15f * (0.5f + 0.5f * sinf(tempoTotal * 3.0f));

    // 3 partes dentro do mesmo corpo
    float margem = 4.0f, espaco = 3.0f;
    float largParte = (corpo.width - 2 * margem - 2 * espaco) / 3.0f;
    float altParte  = corpo.height - 2 * margem;

    for (int i = 0; i < TOKENS_MAX; i++) {
        Rectangle parte = { corpo.x + margem + i * (largParte + espaco), corpo.y + margem, largParte, altParte };
        bool cheia = (i < valor) && !offline;
        // o primeiro token e o da reserva quando ela ja entrou
        Color nivel = (i == 0 && Tokens_ReservaUsada()) ? COR_VERMELHO : COR_AZUL;

        if (cheia) {
            // degrade de cima (rosa) para baixo (azul); a reserva fica toda vermelha
            Color topo = (i == 0 && Tokens_ReservaUsada())
                         ? Misturar(nivel, WHITE, 0.25f)
                         : Misturar(COR_ROSA, WHITE, 0.15f);
            DrawRectangleGradientV((int)parte.x, (int)parte.y, (int)parte.width, (int)parte.height,
                                   Fade(topo, pulso), Fade(nivel, pulso));
            Reflexo(parte, 0.45f);
        } else {
            DrawRectangleRec(parte, Fade(COR_CINZA, 0.30f));
        }

        // perdeu este token: a cor de antes some aos poucos e fica o cinza
        if (animPerda[i] > 0.0f) {
            float t = animPerda[i] / DURACAO_ANIM;           // 1 -> 0
            DrawRectangleRec(parte, Fade(nivel, t));
        }
        // ganhou este token: brilho rosa que cresce e some
        if (animGanho[i] > 0.0f) {
            float t = animGanho[i] / DURACAO_ANIM;
            float extra = (1.0f - t) * 6.0f;
            Rectangle aura = { parte.x - extra, parte.y - extra, parte.width + 2 * extra, parte.height + 2 * extra };
            DrawRectangleLinesEx(aura, 2.5f, Fade(COR_ROSA, t));
        }
    }

    // brilho de vidro por cima de tudo
    Reflexo(corpo, 0.25f);
}

static void Desenha(const char *t, int x, int y, int tam, Color cor, bool forte)
{
    if (forte) UiDrawTextEmphasis(t, x, y, tam, cor); else UiDrawText(t, x, y, tam, cor);
}
// Texto claro com contorno fino e escuro, para ler em qualquer cena (a barra nao tem caixa).
static void TextoContornado(const char *t, float x, float y, int tam, Color cor, bool forte)
{
    int ix = (int)x, iy = (int)y;
    Color borda = Fade((Color){ 20, 22, 60, 255 }, 0.85f);
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if (dx || dy) Desenha(t, ix + dx, iy + dy, tam, borda, forte);
    Desenha(t, ix, iy, tam, cor, forte);
}

void BarraTokens_Desenhar(float x, float y)
{
    int valor = Tokens_Valor();
    bool offline = Tokens_IAOffline();

    // avatar redondo com o simbolo da IA (bolha do MSN)
    Vector2 c = { x + 36, y + BARRA_ALTURA / 2.0f };
    CarregarSimbolo();
    DrawCircleV(c, 26, offline ? COR_CINZA : COR_ROSA);              // aro
    if (offline)
        DrawCircleV(c, 24, (Color){ 120, 124, 140, 255 });
    else
        DrawCircleGradient((Vector2){ c.x, c.y }, 24, COR_AVATAR_CENTRO, COR_AVATAR_BORDA);
    if (simboloIA.id != 0) {
        float lado = 38.0f;
        DrawTexturePro(simboloIA,
                       (Rectangle){ 0, 0, (float)simboloIA.width, (float)simboloIA.height },
                       (Rectangle){ c.x - lado / 2, c.y - lado / 2, lado, lado },
                       (Vector2){ 0, 0 }, 0.0f,
                       offline ? Fade(COR_CLARA, 0.5f) : WHITE);
    } else {
        // sem a imagem, volta para o texto "IA"
        int largIA = UiMeasureText("IA", 20);
        UiDrawText("IA", (int)c.x - largIA / 2, (int)c.y - 8, 20, WHITE);
    }
    DrawEllipse((int)c.x, (int)c.y - 14, 12, 5, Fade(WHITE, 0.25f));

    // status do MSN
    Vector2 st = { c.x + 17, c.y + 17 };
    DrawCircleV(st, 8, WHITE);
    DrawCircleV(st, 6, offline ? COR_CINZA : COR_VERDE);

    // titulo, status e contagem (sem caixa: texto claro com sombra)
    TextoContornado("Tokens da", x + 72, y + 6, 15, WHITE, true);
    TextoContornado("IA", x + 72 + UiMeasureTextEmphasis("Tokens da ", 15), y + 6, 15, offline ? WHITE : (Color){255,150,230,255}, true);
    const char *status = offline ? "offline" : "online";
    TextoContornado(status, x + BARRA_LARGURA - 18 - UiMeasureTextEmphasis(status, 15), y + 6, 15,
       offline ? (Color){200,200,210,255} : (Color){140,230,150,255}, true);

    // bateria unica dividida em 3
    Rectangle corpo = { x + 72, y + 28, 140, 30 };
    DesenharBateria(corpo, valor, offline);

    const char *txt = TextFormat("%d/%d", valor, TOKENS_MAX);
    TextoContornado(txt, corpo.x + corpo.width + 18, corpo.y + 7, 17, WHITE, true);
}
