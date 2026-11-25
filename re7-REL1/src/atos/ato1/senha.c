
// Minijogo da senha do PC da Lia - Ato 1
// Visual: tela de login inspirada no Windows XP ("Doors XP", logo da Vitoria),
// com o perfil da Lia, o campo de senha e a seta verde para confirmar.
// Etapa 1: escolher os quatro caracteres que respondem as pistas dos fragmentos.
// Etapa 2: arrastar os caracteres para a ordem certa usando as pistas de posicao.
// A IA so oferece ajuda na etapa 2, a cada SENHA_ERROS_RODADA erros, sem aviso
// previo na tela, numa janela de MSN igual a do Ato 2. A ajuda gasta 1 token
// (o custo nao aparece na tela, como no Ato 2) e revela a 1a e a 3a posicoes.

#include "senha.h"
#include "tokens.h"
#include "ui.h"
#include "raylib.h"
#include <string.h>

#define COR_MARINHO  (Color){29, 38, 96, 255}
#define COR_ROSA     (Color){227, 93, 215, 255}
#define COR_ROSA_HOV (Color){255, 105, 200, 255}   /* rosa do hover das escolhas do Ato 2 */
#define COR_ROSA_TXT (Color){255, 190, 235, 255}
#define COR_AZUL     (Color){39, 172, 234, 255}
#define COR_VERDE    (Color){76, 175, 80, 255}
#define COR_CLARA    (Color){232, 240, 250, 255}
#define COR_PAINEL   (Color){0, 40, 122, 255}   /* fundo do painel de dicas */

/* Cores da tela de login */
#define XP_AZUL_ESC  (Color){0, 48, 150, 255}
#define XP_AZUL      (Color){90, 126, 220, 255}
#define XP_AZUL_CLA  (Color){130, 162, 236, 255}
#define XP_LARANJA   (Color){245, 160, 40, 255}
#define XP_BORDA     (Color){40, 70, 160, 255}
#define XP_BOTAO     (Color){244, 246, 252, 255}

typedef enum {
    ETAPA_CARACTERES,
    ETAPA_ORDEM,
    ETAPA_POPUP_IA,
    ETAPA_DICA_IA,
    ETAPA_ACERTOU,
    ETAPA_FIM
} EstadoSenha;

static const char SENHA[SENHA_TAMANHO + 1] = "Y2K7";

/* Caracteres corretos da etapa 1 (a ordem nao importa nesta etapa). */
static const char RESPOSTAS_FRAGMENTOS[] = {'K', 'Y', '7', '2'};

/* Oito opcoes da etapa 1: quatro corretas e quatro distratoras. */
static const char OPCOES_FRAGMENTOS[] = {'A', '7', 'M', 'Y', '2', '9', 'K', '0'};

/* Na etapa 2, as pecas aparecem nesta ordem: K, Y, 7, 2. */
static const char PECAS_ORDEM[] = {'K', 'Y', '7', '2'};

static EstadoSenha estado;
static int etapa;
static char caracteresFragmentos[4];
static char ordem[4];
static bool posicaoRevelada[4];
static int erros;
static bool ajudaUsada;
static bool mostrarDicaIA;
static float tempoAviso;
static bool avisoIncompleto;   /* aviso de "falta preencher", que nao conta como erro */
static float tempoAcerto;

/* Arraste da etapa 2 */
static bool arrastando;
static char arrasteCaractere;
static int arrasteOrigem;      /* -1: veio da bandeja; 0 a 3: veio de uma posicao */
static int pecaPressionada = -1;  /* peca da bandeja sob o botao do mouse, ou -1 */
static Vector2 pontoPressionado;  /* onde o botao foi pressionado */
#define DISTANCIA_ARRASTE 6.0f    /* menos que isso e clique; mais que isso e arraste */

/* Dicas da etapa 2 (indices na ordem de PECAS_ORDEM) */
static int dicaSelecionada = 0;   /* dica aberta no painel */
static int pecaDestacada = 0;     /* peca em rosa, ou -1; arrastar a peca tira o rosa */

/* Imagens: carregadas no primeiro desenho (precisam da janela aberta) */
static Texture2D texLogo, texLia, texIA;
static bool texturasCarregadas = false;

/* Janela de MSN da IA (mesmo formato da escolha da IA no Ato 2) */
static const Rectangle JANELA_MSN = {330, 170, 620, 330};

static Rectangle BotaoMSN(int i)
{
    return (Rectangle){370.0f, 320.0f + i * 80.0f, 540.0f, 62.0f};
}

static const Rectangle SETA_ETAPA1 = {1054, 236, 42, 42};
static const Rectangle SETA_ETAPA2 = {806, 209, 48, 48};

static Rectangle RetanguloOpcao(int i)
{
    return (Rectangle){742.0f + (i % 4) * 92.0f, 334.0f + (i / 4) * 70.0f, 78.0f, 56.0f};
}

static Rectangle RetanguloPosicao(int pos)
{
    return (Rectangle){424.0f + pos * 92.0f, 201.0f, 78.0f, 64.0f};
}

static Rectangle RetanguloPeca(int i)
{
    return (Rectangle){411.0f + i * 119.0f, 328.0f, 100.0f, 66.0f};
}

/* ------------------------- TEXTO (fonte Nunito do ui.c) ------------------------- */

static void Texto(const char *texto, float x, float y, int tamanho, Color cor)
{
    UiDrawText(texto, (int)x, (int)y, tamanho, cor);
}

static void TextoForte(const char *texto, float x, float y, int tamanho, Color cor)
{
    UiDrawTextEmphasis(texto, (int)x, (int)y, tamanho, cor);
}

static void Centro(const char *texto, float centroX, float y, int tamanho, Color cor)
{
    UiDrawText(texto, (int)(centroX - UiMeasureText(texto, tamanho) / 2.0f),
               (int)y, tamanho, cor);
}

static void CentroForte(const char *texto, float centroX, float y, int tamanho, Color cor)
{
    UiDrawTextEmphasis(texto,
                       (int)(centroX - UiMeasureTextEmphasis(texto, tamanho) / 2.0f),
                       (int)y, tamanho, cor);
}

static bool Clique(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static Texture2D CarregarComMipmaps(const char *caminho)
{
    Texture2D t = LoadTexture(caminho);
    if (t.id != 0) {
        GenTextureMipmaps(&t);
        SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
    }
    return t;
}

static void CarregarTexturas(void)
{
    if (texturasCarregadas) return;
    texturasCarregadas = true;
    texLogo = CarregarComMipmaps("assets/ato1/ato1_logo_doorsxp.png");
    texLia  = CarregarComMipmaps("assets/comum/lia07_neutra.png");
    texIA   = CarregarComMipmaps("assets/comum/iaStar.png");
}

void Senha_Descarregar(void)
{
    if (!texturasCarregadas) return;
    if (texLogo.id) UnloadTexture(texLogo);
    if (texLia.id)  UnloadTexture(texLia);
    if (texIA.id)   UnloadTexture(texIA);
    texLogo = texLia = texIA = (Texture2D){0};
    texturasCarregadas = false;
}

static void DesenharTextura(Texture2D t, Rectangle origem, Rectangle destino)
{
    if (t.id != 0) DrawTexturePro(t, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
}

/* ------------------------- TELA DE LOGIN ------------------------- */

static void Fundo(void)
{
    DrawRectangle(0, 0, 1280, 720, XP_AZUL);
    /* brilho: atras da logo na etapa 1, centralizado na etapa 2 */
    if (etapa == 1)
        DrawCircleGradient((Vector2){260, 170}, 520, Fade(XP_AZUL_CLA, .9f), Fade(XP_AZUL, 0));
    else
        DrawCircleGradient((Vector2){640, 260}, 640, Fade(XP_AZUL_CLA, .9f), Fade(XP_AZUL, 0));

    /* faixas de cima e de baixo */
    DrawRectangle(0, 0, 1280, 86, XP_AZUL_ESC);
    DrawRectangleGradientH(0, 86, 640, 3, Fade(WHITE, 0), WHITE);
    DrawRectangleGradientH(640, 86, 640, 3, WHITE, Fade(WHITE, 0));
    DrawRectangle(0, 634, 1280, 86, XP_AZUL_ESC);
    DrawRectangleGradientH(0, 631, 640, 3, Fade(XP_LARANJA, 0), XP_LARANJA);
    DrawRectangleGradientH(640, 631, 640, 3, XP_LARANJA, Fade(XP_LARANJA, 0));

    /* O separador vertical existe apenas na etapa 1. Na etapa 2 ele atravessava
       a tela e ficava visível atrás do painel de pistas, fora do layout da referência. */
    if (etapa == 1) {
        DrawRectangleGradientV(560, 130, 2, 230, Fade(WHITE, 0), Fade(WHITE, .8f));
        DrawRectangleGradientV(560, 360, 2, 230, Fade(WHITE, .8f), Fade(WHITE, 0));
    }

    TextoForte("PC da", 40, 28, 26, WHITE);
    TextoForte("LIA", 40 + UiMeasureTextEmphasis("PC da ", 26), 28, 26, COR_ROSA_TXT);

    /* rodape: so a frase. O canto esquerdo fica livre para a engrenagem de pausa do game.c. */
    Centro("Depois de entrar, a IA estará esperando por você.", 640, 666, 20, Fade(WHITE, .85f));
}

/* Lado esquerdo: logo, boas-vindas e as pistas em caixinhas */
static void LadoPistas(const char *subtitulo, const char **linhas, int quantidade)
{
    if (texLogo.id != 0) {
        float altura = 128.0f * texLogo.height / texLogo.width;
        DesenharTextura(texLogo, (Rectangle){0, 0, (float)texLogo.width, (float)texLogo.height},
                        (Rectangle){90, 120, 128, altura});
    }
    TextoForte("Bem-vinda", 236, 150, 38, WHITE);
    if (strcmp(subtitulo, "Descubra os quatro caracteres da sua senha.") == 0) {
    Texto("Descubra os quatro caracteres", 238, 198, 20, Fade(WHITE, .9f));
    Texto("da sua senha.", 238, 224, 20, Fade(WHITE, .9f));
    } else {
    Texto("Agora organize os caracteres", 238, 198, 20, Fade(WHITE, .9f));
    Texto("na ordem certa.", 238, 224, 20, Fade(WHITE, .9f));
    }

    for (int i = 0; i < quantidade; i++) {
        Rectangle r = {90, 290.0f + i * 76.0f, 420, 66};
        DrawRectangleRounded(r, .18f, 8, Fade(XP_AZUL_ESC, .45f));
        DrawRectangleRoundedLinesEx(r, .18f, 8, 1.2f, Fade(WHITE, .5f));
        Texto(linhas[2 * i], r.x + 16, r.y + 8, 18, COR_ROSA_TXT);
        Texto(linhas[2 * i + 1], r.x + 16, r.y + 30, 23, WHITE);
    }
}

/* Lado direito: foto e nome da usuaria */
static void Perfil(void)
{
    Rectangle foto = {620, 150, 96, 96};
    DrawRectangleRounded((Rectangle){foto.x - 4, foto.y - 4, foto.width + 8, foto.height + 8},
                         .18f, 8, XP_LARANJA);
    DrawRectangleRounded((Rectangle){foto.x - 1, foto.y - 1, foto.width + 2, foto.height + 2},
                         .16f, 8, WHITE);
    /* recorte do rosto da Lia de 2007 */
    DesenharTextura(texLia, (Rectangle){170, 140, 660, 660}, foto);

    TextoForte("Lia", 740, 160, 34, WHITE);
    Texto("Digite sua senha:", 742, 206, 22, Fade(WHITE, .9f));
}

static void SetaVerde(Rectangle b)
{
    bool sobre = CheckCollisionPointRec(GetMousePosition(), b);
    DrawRectangleRounded(b, .25f, 6, sobre ? (Color){80, 200, 80, 255} : (Color){60, 170, 60, 255});
    DrawRectangleRoundedLinesEx(b, .25f, 6, 2, WHITE);
    float k = b.width / 42.0f;
    DrawTriangle((Vector2){b.x + 30 * k, b.y + 21 * k}, (Vector2){b.x + 16 * k, b.y + 10 * k},
                 (Vector2){b.x + 16 * k, b.y + 32 * k}, WHITE);
    DrawRectangleRec((Rectangle){b.x + 9 * k, b.y + 17 * k, 10 * k, 8 * k}, WHITE);
}

static void BotaoCaractere(Rectangle r, char caractere, bool selecionado, bool usado)
{
    bool sobre = !usado && CheckCollisionPointRec(GetMousePosition(), r);
    Color fundo = usado ? Fade(WHITE, .25f) :
                  selecionado ? COR_ROSA :
                  sobre ? (Color){255, 225, 246, 255} : XP_BOTAO;
    DrawRectangleRounded(r, .18f, 6, fundo);
    DrawRectangleRoundedLinesEx(r, .18f, 6, 1.5f, selecionado ? WHITE : XP_BORDA);
    char s[2] = {caractere, '\0'};
    CentroForte(s, r.x + r.width / 2, r.y + (r.height - 28) / 2, 28,
                usado ? Fade(WHITE, .6f) : selecionado ? WHITE : COR_MARINHO);
}

/* ------------------------- REGRAS ------------------------- */


static void LimparFragmentos(void)
{
    memset(caracteresFragmentos, 0, sizeof(caracteresFragmentos));
}

static void LimparOrdem(void)
{
    for (int i = 0; i < 4; i++) {
        if (!posicaoRevelada[i]) ordem[i] = '\0';
    }
    arrastando = false;
    pecaPressionada = -1;
}

static bool CaractereJaUsadoNaEtapa1(char c)
{
    for (int i = 0; i < 4; i++)
        if (caracteresFragmentos[i] == c) return true;
    return false;
}

static bool CaractereJaUsadoNaEtapa2(char c)
{
    if (arrastando && arrasteCaractere == c) return true;
    for (int i = 0; i < 4; i++)
        if (ordem[i] == c) return true;
    return false;
}

static bool FragmentosCorretos(void)
{
    /* A selecao e validada como conjunto: a ordem dos cliques nao importa. */
    for (int i = 0; i < 4; i++) {
        bool encontrado = false;
        for (int j = 0; j < 4; j++) {
            if (caracteresFragmentos[j] == RESPOSTAS_FRAGMENTOS[i]) {
                encontrado = true;
                break;
            }
        }
        if (!encontrado) return false;
    }
    return true;
}

static int QuantidadeSelecionadaEtapa1(void)
{
    int quantidade = 0;
    for (int i = 0; i < 4; i++)
        if (caracteresFragmentos[i] != '\0') quantidade++;
    return quantidade;
}

static bool OrdemCorreta(void)
{
    for (int i = 0; i < 4; i++)
        if (ordem[i] != SENHA[i]) return false;
    return true;
}

static bool OrdemPreenchida(void)
{
    for (int i = 0; i < 4; i++)
        if (!ordem[i]) return false;
    return true;
}

static void AvisarIncompleto(void)
{
    /* Confirmar sem completar nao conta como erro. */
    avisoIncompleto = true;
    tempoAviso = 1.8f;
}

static void ConfirmarFragmentos(void)
{
    if (QuantidadeSelecionadaEtapa1() < 4) {
        AvisarIncompleto();
        return;
    }

    if (FragmentosCorretos()) {
        etapa = 2;
        estado = ETAPA_ORDEM;
        erros = 0;
        tempoAviso = 0.0f;
        LimparOrdem();
    } else {
        /* A etapa 1 nao tem contador de erros nem oferta de ajuda da IA. */
        LimparFragmentos();
        avisoIncompleto = false;
        tempoAviso = 2.0f;
    }
}

static void RegistrarErroEtapa2(void)
{
    avisoIncompleto = false;
    tempoAviso = 1.6f;
    LimparOrdem();

    /* Depois da ajuda, os erros deixam de ser contados. */
    if (!ajudaUsada) {
        erros++;
        if (erros >= SENHA_ERROS_RODADA)
            estado = ETAPA_POPUP_IA;
    }
}

static void ConfirmarOrdem(void)
{
    if (!OrdemPreenchida()) {
        AvisarIncompleto();
        return;
    }

    if (OrdemCorreta()) {
        estado = ETAPA_ACERTOU;
        tempoAcerto = 2.0f;
    } else {
        RegistrarErroEtapa2();
    }
}

static void AceitarAjudaIA(void)
{
    if (Tokens_Valor() <= 0) {
        tempoAviso = 2.0f;
        return;
    }

    Tokens_Gastar();
    ajudaUsada = true;
    mostrarDicaIA = true;
    erros = 0;

    /* A IA revela a primeira e a terceira posicoes da senha. */
    ordem[0] = 'Y';
    ordem[2] = 'K';
    posicaoRevelada[0] = true;
    posicaoRevelada[2] = true;
    if (pecaDestacada >= 0 && CaractereJaUsadoNaEtapa2(PECAS_ORDEM[pecaDestacada]))
        pecaDestacada = -1;

    /* Se Y ou K estavam em outra posicao, saem de la. */
    for (int i = 0; i < 4; i++) {
        if (!posicaoRevelada[i] && (ordem[i] == 'Y' || ordem[i] == 'K'))
            ordem[i] = '\0';
    }

    arrastando = false;
    estado = ETAPA_DICA_IA;
}

static void RecusarAjudaIA(void)
{
    /* Recusar nao consome tokens; a oferta volta depois de mais SENHA_ERROS_RODADA erros. */
    erros = 0;
    estado = ETAPA_ORDEM;
    tempoAviso = 0.0f;
}

void Senha_ComecarJogo(void)
{
    ajudaUsada = false;
    mostrarDicaIA = false;
    Senha_Iniciar();
}

void Senha_Iniciar(void)
{
    /* A etapa 2 abre com a dica do K. */
    dicaSelecionada = 0;
    pecaDestacada = 0;
    pecaPressionada = -1;
    estado = ETAPA_CARACTERES;
    etapa = 1;
    erros = 0;
    tempoAviso = 0.0f;
    avisoIncompleto = false;
    tempoAcerto = 0.0f;
    memset(posicaoRevelada, 0, sizeof(posicaoRevelada));
    memset(ordem, 0, sizeof(ordem));
    LimparFragmentos();
    arrastando = false;
}

/* ------------------------- ATUALIZACAO ------------------------- */

static void AtualizarEtapa1(void)
{
    /* A jogadora marca quatro dos oito caracteres; clicar de novo desmarca. */
    for (int i = 0; i < 8; i++) {
        if (!Clique(RetanguloOpcao(i))) continue;

        char c = OPCOES_FRAGMENTOS[i];
        int encontrado = -1;
        for (int j = 0; j < 4; j++) {
            if (caracteresFragmentos[j] == c) {
                encontrado = j;
                break;
            }
        }

        if (encontrado >= 0) {
            caracteresFragmentos[encontrado] = '\0';
        } else if (QuantidadeSelecionadaEtapa1() < 4) {
            for (int j = 0; j < 4; j++) {
                if (caracteresFragmentos[j] == '\0') {
                    caracteresFragmentos[j] = c;
                    break;
                }
            }
        }
        break;
    }

    if (Clique(SETA_ETAPA1) || IsKeyPressed(KEY_ENTER))
        ConfirmarFragmentos();
}

static void AtualizarEtapa2(void)
{
    Vector2 mouse = GetMousePosition();

    /* Pressionar uma peca ainda nao decide nada: clique consulta a dica, arraste
       leva para a senha. Pegar de uma posicao ja preenchida arrasta na hora. */
    if (!arrastando && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        pecaPressionada = -1;
        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(mouse, RetanguloPeca(i))) {
                pecaPressionada = i;
                pontoPressionado = mouse;
                break;
            }
        }
        for (int pos = 0; pos < 4 && !arrastando; pos++) {
            if (CheckCollisionPointRec(mouse, RetanguloPosicao(pos)) &&
                ordem[pos] && !posicaoRevelada[pos]) {
                arrastando = true;
                arrasteCaractere = ordem[pos];
                arrasteOrigem = pos;
                ordem[pos] = '\0';
            }
        }
    }

    if (pecaPressionada >= 0) {
        float dx = mouse.x - pontoPressionado.x;
        float dy = mouse.y - pontoPressionado.y;
        bool moveu = dx * dx + dy * dy > DISTANCIA_ARRASTE * DISTANCIA_ARRASTE;

        if (moveu) {
            /* Passou da distancia minima: vira arraste (peca ja usada nao arrasta). */
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
                !CaractereJaUsadoNaEtapa2(PECAS_ORDEM[pecaPressionada])) {
                arrastando = true;
                arrasteCaractere = PECAS_ORDEM[pecaPressionada];
                arrasteOrigem = -1;
            }
            pecaPressionada = -1;
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            /* Clique: mostra a dica (tambem nas pecas ja usadas). */
            dicaSelecionada = pecaPressionada;
            pecaDestacada = pecaPressionada;
            pecaPressionada = -1;
        } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            pecaPressionada = -1;   /* botao solto fora desta tela (pausa, por exemplo) */
        }
    }

    /* Arrastar a peca em rosa tira o destaque: ela fica cinza como as outras.
       A dica continua aberta, e um novo clique devolve o rosa. */
    if (arrastando && pecaDestacada >= 0 && PECAS_ORDEM[pecaDestacada] == arrasteCaractere)
        pecaDestacada = -1;

    /* Solta a peca. */
    if (arrastando && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        int alvo = -1;
        for (int pos = 0; pos < 4; pos++)
            if (CheckCollisionPointRec(mouse, RetanguloPosicao(pos))) alvo = pos;

        if (alvo >= 0 && !posicaoRevelada[alvo]) {
            char anterior = ordem[alvo];
            ordem[alvo] = arrasteCaractere;
            /* Soltar sobre outra peca troca as duas de lugar; se a peca veio da
               bandeja, a anterior volta para a bandeja. */
            if (anterior && arrasteOrigem >= 0) ordem[arrasteOrigem] = anterior;
        } else if (alvo >= 0 && arrasteOrigem >= 0) {
            /* Posicao revelada pela IA: a peca volta para onde estava. */
            ordem[arrasteOrigem] = arrasteCaractere;
        }
        /* Soltar fora das posicoes: a peca volta para a bandeja. */
        arrastando = false;
    }

    if (!arrastando && (Clique(SETA_ETAPA2) || IsKeyPressed(KEY_ENTER)))
        ConfirmarOrdem();
}


void Senha_Atualizar(void)
{
    float dt = GetFrameTime();
    if (tempoAviso > 0.0f) tempoAviso -= dt;

    switch (estado) {
    case ETAPA_CARACTERES:
        AtualizarEtapa1();
        break;

    case ETAPA_ORDEM:
        AtualizarEtapa2();
        break;

    case ETAPA_POPUP_IA:
        if (Tokens_Valor() > 0) {
            if (Clique(BotaoMSN(0)) || IsKeyPressed(KEY_ONE))
                AceitarAjudaIA();
            else if (Clique(BotaoMSN(1)) || IsKeyPressed(KEY_TWO))
                RecusarAjudaIA();
        } else {
            /* sem tokens: so da para continuar sozinha */
            if (Clique(BotaoMSN(1)) || IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_ENTER))
                RecusarAjudaIA();
        }
        break;

    case ETAPA_DICA_IA:
        if (Clique(BotaoMSN(1)) || IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_ENTER))
            estado = ETAPA_ORDEM;
        break;

    case ETAPA_ACERTOU:
        tempoAcerto -= dt;
        if (tempoAcerto <= 0.0f) estado = ETAPA_FIM;
        break;

    case ETAPA_FIM:
        break;
    }
}

/* ------------------------- DESENHO ------------------------- */

static void Aviso(void)
{
    if (tempoAviso <= 0.0f) return;
    const char *msg;
    if (etapa == 1)
        msg = avisoIncompleto ? "Selecione os quatro caracteres antes de confirmar."
                              : "Os caracteres não correspondem aos fragmentos. Tente de novo.";
    else
        msg = avisoIncompleto ? "Preencha as quatro posições antes de confirmar."
                              : "Senha incorreta. Revise as pistas e tente de novo.";
    /* Na etapa 2 o balao fica ao lado da seta, sem cobrir o painel de dicas. */
    Rectangle r = {742, 520, 470, 40};
    int tamanho = 20;
    if (etapa == 2) {
        tamanho = 18;
        r = (Rectangle){870, 213, (float)UiMeasureText(msg, tamanho) + 28, 40};
    }
    DrawRectangleRounded(r, .3f, 8, (Color){255, 255, 225, 255});         /* balao amarelo do XP */
    DrawRectangleRoundedLinesEx(r, .3f, 8, 1.2f, (Color){120, 100, 40, 255});
    Texto(msg, r.x + 14, r.y + (r.height - tamanho) / 2 - 1, tamanho, COR_MARINHO);
}

static void DesenharEtapa1(void)
{
    const char *pistas[] = {
        "FRAGMENTO A", "Inicial de Kelly Key",
        "FRAGMENTO B", "Último algarismo de 2007",
        "FRAGMENTO C", "Inicial da palavra Year",
        "FRAGMENTO D", "Primeiro algarismo de 2007"
    };
    LadoPistas("Descubra os quatro caracteres da sua senha.", pistas, 4);
    Perfil();

    /* campo de senha: mostra os caracteres escolhidos, na ordem dos cliques */
    Rectangle campo = {742, 236, 300, 40};
    DrawRectangleRounded(campo, .15f, 6, WHITE);
    DrawRectangleRoundedLinesEx(campo, .15f, 6, 1.5f, (Color){60, 90, 180, 255});
    for (int i = 0; i < 4; i++) {
        char s[2] = {caracteresFragmentos[i] ? caracteresFragmentos[i] : '?', '\0'};
        TextoForte(s, campo.x + 16 + i * 30, campo.y + 7, 24,
                   caracteresFragmentos[i] ? COR_MARINHO : (Color){150, 170, 220, 255});
    }
    SetaVerde(SETA_ETAPA1);

    Texto("Escolha quatro caracteres:", 742, 300, 22, Fade(WHITE, .9f));
    for (int i = 0; i < 8; i++)
        BotaoCaractere(RetanguloOpcao(i), OPCOES_FRAGMENTOS[i],
                       CaractereJaUsadoNaEtapa1(OPCOES_FRAGMENTOS[i]), false);

    Texto("Etapa 1 de 2", 742, 486, 19, Fade(WHITE, .85f));
    Aviso();
}

/* Texto com marcacao simples: {conectivo em rosa} e '*' como ponto separador (desenhado).
   Devolve a largura; com desenhar = false, apenas mede. */
static float TextoRico(const char *texto, float x, float y, int tamanho, Color cor, bool desenhar)
{
    char trecho[160];
    int n = 0;
    float inicio = x;
    bool destaque = false;

    for (const char *c = texto;; c++) {
        bool especial = *c == '\0' || *c == '{' || *c == '}' || *c == '*';
        if (!especial) {
            if (n < (int)sizeof(trecho) - 1) trecho[n++] = *c;
            continue;
        }

        if (n > 0) {
            trecho[n] = '\0';
            if (destaque) {
                if (desenhar) TextoForte(trecho, x, y, tamanho, COR_ROSA_TXT);
                x += UiMeasureTextEmphasis(trecho, tamanho);
            } else {
                if (desenhar) Texto(trecho, x, y, tamanho, cor);
                x += UiMeasureText(trecho, tamanho);
            }
            n = 0;
        }

        if (*c == '\0') break;
        if (*c == '{') destaque = true;
        else if (*c == '}') destaque = false;
        else {
            float largura = tamanho * .30f;
            if (desenhar)
                DrawCircleV((Vector2){x + largura / 2, y + tamanho * .53f}, 1.8f, cor);
            x += largura;
        }
    }
    return x - inicio;
}

static void CentroRico(const char *texto, float centroX, float y, int tamanho, Color cor)
{
    float largura = TextoRico(texto, 0, 0, tamanho, cor, false);
    TextoRico(texto, centroX - largura / 2, y, tamanho, cor, true);
}

/* Desenha os simbolos logicos com primitivas para nao depender de glyphs da fonte.
   'n' negacao, 'a' conjuncao, 'o' disjuncao, 'i' condicional. */
static void SimboloLogico(char tipo, float cx, float cy, Color cor)
{
    const float w = 12, h = 15, t = 3;
    if (tipo == 'n') {
        DrawLineEx((Vector2){cx - 17, cy - 8}, (Vector2){cx + 17, cy - 8}, t, cor);
        DrawLineEx((Vector2){cx + 16, cy - 9}, (Vector2){cx + 16, cy + 8}, t, cor);
    } else if (tipo == 'a') {
        DrawLineEx((Vector2){cx - w, cy + h}, (Vector2){cx, cy - h}, t, cor);
        DrawLineEx((Vector2){cx, cy - h}, (Vector2){cx + w, cy + h}, t, cor);
    } else if (tipo == 'o') {
        DrawLineEx((Vector2){cx - w, cy - h}, (Vector2){cx, cy + h}, t, cor);
        DrawLineEx((Vector2){cx, cy + h}, (Vector2){cx + w, cy - h}, t, cor);
    } else if (tipo == 'i') {
        DrawLineEx((Vector2){cx - 18, cy}, (Vector2){cx + 18, cy}, t, cor);
        DrawLineEx((Vector2){cx + 8, cy - 10}, (Vector2){cx + 18, cy}, t, cor);
        DrawLineEx((Vector2){cx + 8, cy + 10}, (Vector2){cx + 18, cy}, t, cor);
    }
}

typedef struct {
    const char *titulo;
    const char *operador;
    char simbolo;
    const char *p;
    const char *q;        /* NULL na negacao, que so tem uma proposicao */
    const char *letraP;
    const char *letraQ;
    const char *leitura;
} DicaLogica;

/* Uma dica por peca, na mesma ordem de PECAS_ORDEM: K, Y, 7, 2. */
static const DicaLogica DICAS[4] = {
    {"Dica do K", "Conjunção", 'a', "Y vem antes de K", "2 vem antes de 7", "p", "q",
     "Lê-se: Y vem antes de K {E} 2 vem antes de 7."},
    {"Dica do Y", "Negação", 'n', "K vem antes de 2", NULL, "r", NULL,
     "Lê-se: {NÃO} é verdade que K vem antes de 2."},
    {"Dica do 7", "Disjunção", 'o', "Y está na 1ª posição", "7 está na 1ª posição", "s", "t",
     "Lê-se: Y está na 1ª posição {OU} 7 está na 1ª posição."},
    {"Dica do 2", "Condicional", 'i', "2 está na 2ª posição", "K está na 3ª posição", "u", "v",
     "Lê-se: {SE} 2 está na 2ª posição, {ENTÃO} K está na 3ª posição."}
};

/* Cores: rosa = clicada; cinza = ja esta na senha; branca = ainda nao colocada. */
static void PecaEtapa2(int i)
{
    Rectangle r = RetanguloPeca(i);
    bool usada = CaractereJaUsadoNaEtapa2(PECAS_ORDEM[i]);
    bool selecionada = pecaDestacada == i;
    bool sobre = estado == ETAPA_ORDEM && !arrastando &&
                 CheckCollisionPointRec(GetMousePosition(), r);

    Color fundo = selecionada ? COR_ROSA :
                  usada ? (sobre ? (Color){216, 219, 226, 255} : (Color){198, 202, 212, 255}) :
                  sobre ? (Color){255, 225, 246, 255} : WHITE;
    Color corTexto = selecionada ? WHITE : usada ? (Color){92, 98, 122, 255} : COR_MARINHO;

    DrawRectangleRounded(r, .24f, 8, fundo);
    DrawRectangleRoundedLinesEx(r, .24f, 8, selecionada ? 3.0f : 2.0f,
                                selecionada ? WHITE : XP_BORDA);
    char s[2] = {PECAS_ORDEM[i], '\0'};
    CentroForte(s, r.x + r.width / 2, r.y + 16, 34, corTexto);
}

#define PROP_TAMANHO 27
#define PROP_ALTURA  54.0f

static float LarguraProposicao(const char *texto)
{
    return UiMeasureTextEmphasis(texto, PROP_TAMANHO) + 44.0f;
}

/* Caixa branca da proposicao, com a letra numa etiqueta de cantos arredondados. */
static void CaixaProposicao(const char *texto, const char *letra, float x, float y)
{
    Rectangle r = {x, y, LarguraProposicao(texto), PROP_ALTURA};
    DrawRectangleRounded(r, .24f, 8, WHITE);
    CentroForte(texto, r.x + r.width / 2, r.y + 13, PROP_TAMANHO, COR_MARINHO);
    if (letra != NULL) {
        float cx = r.x + 6, cy = r.y - 3;
        DrawRectangleRounded((Rectangle){cx - 16, cy - 16, 32, 32}, .42f, 8, WHITE);
        DrawRectangleRounded((Rectangle){cx - 14.5f, cy - 14.5f, 29, 29}, .40f, 8, COR_PAINEL);
        Centro(letra, cx, cy - 11, 20, WHITE);
    }
}

static void PainelDica(void)
{
    const DicaLogica *d = &DICAS[dicaSelecionada];
    Rectangle painel = {202, 413, 876, 206};
    Color borda = {205, 222, 255, 255};
    float centroX = painel.x + painel.width / 2;

    DrawTriangle((Vector2){centroX, 403}, (Vector2){centroX - 12, 414},
                 (Vector2){centroX + 12, 414}, borda);
    DrawRectangleRounded(painel, .16f, 10, COR_PAINEL);
    DrawRectangleRoundedLinesEx(painel, .16f, 10, 2, borda);

    TextoForte(d->titulo, 236, 430, 29, WHITE);
    Texto(d->operador, 236 + UiMeasureTextEmphasis(d->titulo, 29) + 27, 431, 27, WHITE);

    float y = 489;
    float centroY = y + PROP_ALTURA / 2;
    float larguraP = LarguraProposicao(d->p);
    if (d->q == NULL) {
        float x = centroX - (34 + 32 + larguraP) / 2;
        SimboloLogico(d->simbolo, x + 17, centroY, WHITE);
        CaixaProposicao(d->p, d->letraP, x + 34 + 32, y);
    } else {
        const float vao = 110;
        float x = centroX - (larguraP + vao + LarguraProposicao(d->q)) / 2;
        CaixaProposicao(d->p, d->letraP, x, y);
        /* a etiqueta da segunda caixa invade o vao; o simbolo fica no meio do espaco livre */
        SimboloLogico(d->simbolo, x + larguraP + vao / 2 - 4, centroY, WHITE);
        CaixaProposicao(d->q, d->letraQ, x + larguraP + vao, y);
    }

    CentroRico(d->leitura, centroX, 566, 27, Fade(WHITE, .95f));
}

static void DesenharEtapa2(void)
{
    /* Cabecalho centralizado: logo, boas-vindas e foto. */
    const char *titulo = "Bem-vinda, Lia";
    float larguraTitulo = (float)UiMeasureTextEmphasis(titulo, 33);
    float x = 640 - (66 + 21 + larguraTitulo + 25 + 60) / 2;
    if (texLogo.id != 0) {
        float altura = 66.0f * texLogo.height / texLogo.width;
        DesenharTextura(texLogo, (Rectangle){0, 0, (float)texLogo.width, (float)texLogo.height},
                        (Rectangle){x, 128 - altura / 2, 66, altura});
    }
    x += 66 + 21;
    TextoForte(titulo, x, 110, 33, WHITE);
    x += larguraTitulo + 25;
    Rectangle foto = {x, 98, 60, 60};
    DrawRectangleRounded((Rectangle){foto.x - 3, foto.y - 3, foto.width + 6, foto.height + 6},
                         .2f, 8, XP_LARANJA);
    DrawRectangleRounded(foto, .16f, 8, WHITE);
    DesenharTextura(texLia, (Rectangle){170, 140, 660, 660}, foto);

    Centro("Deixe as quatro dicas verdadeiras.", 640, 168, 23, Fade(WHITE, .95f));

    /* Quatro posicoes separadas e a seta de confirmar. */
    for (int i = 0; i < 4; i++) {
        Rectangle r = RetanguloPosicao(i);
        char s[2] = {ordem[i] ? ordem[i] : '?', '\0'};
        DrawRectangleRounded(r, .22f, 8, WHITE);
        DrawRectangleRoundedLinesEx(r, .22f, 8, 1.5f, XP_BORDA);
        CentroForte(s, r.x + r.width / 2, r.y + 15, 34,
                    ordem[i] ? COR_MARINHO : (Color){150, 170, 220, 255});
    }
    SetaVerde(SETA_ETAPA2);

    CentroRico("Clique para ver a dica * arraste para a senha", 640, 287, 22, Fade(WHITE, .92f));
    for (int i = 0; i < 4; i++) PecaEtapa2(i);
    PainelDica();
    Texto("Etapa 2 de 2", 1096, 596, 19, Fade(WHITE, .85f));

    Aviso();
    if (arrastando) {
        Vector2 mouse = GetMousePosition();
        Rectangle r = {mouse.x - 39, mouse.y - 32, 78, 64};
        DrawRectangleRounded((Rectangle){r.x + 4, r.y + 6, r.width, r.height}, .16f, 8, Fade(BLACK, .3f));
        BotaoCaractere(r, arrasteCaractere, true, false);
    }
}

/* Janela de MSN da IA, no mesmo formato da escolha da IA no Ato 2 */
static void JanelaMSN(void)
{
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .45f));
    Rectangle j = JANELA_MSN;
    DrawRectangleRounded(j, .04f, 8, (Color){232, 240, 250, 250});
    DrawRectangleRounded((Rectangle){j.x, j.y, j.width, 44}, .3f, 8, COR_AZUL);
    DrawRectangleRoundedLinesEx(j, .04f, 8, 1.5f, COR_MARINHO);
    DrawCircle((int)j.x + 26, (int)j.y + 22, 8, COR_VERDE);
    Texto("IA diz:", j.x + 44, j.y + 8, 24, WHITE);
    if (texIA.id != 0)
        DesenharTextura(texIA, (Rectangle){0, 0, (float)texIA.width, (float)texIA.height},
                        (Rectangle){j.x + 24, j.y + 64, 56, 56});
}

static void BotaoEscolha(int i, const char *rotulo)
{
    /* igual as opcoes do Ato 2: azul-marinho, rosa com o mouse por cima */
    Rectangle r = BotaoMSN(i);
    bool sobre = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, .3f, 8, sobre ? COR_ROSA_HOV : (Color){29, 38, 96, 235});
    DrawRectangleRoundedLines(r, .3f, 8, WHITE);
    int largura = UiMeasureText(rotulo, 28);
    Texto(rotulo, r.x + (r.width - largura) / 2, r.y + 15, 28, WHITE);
}

static void DesenharPopupIA(void)
{
    JanelaMSN();
    Rectangle j = JANELA_MSN;
    /* SENHA_ERROS_RODADA vale 3. Se mudar, ajuste o texto abaixo. */
    Texto("Você errou três vezes.", j.x + 96, j.y + 68, 28, COR_MARINHO);
    if (Tokens_Valor() > 0) {
        Texto("Quer uma ajuda?", j.x + 96, j.y + 102, 28, COR_MARINHO);
        BotaoEscolha(0, "Usar a ajuda");
        BotaoEscolha(1, "Continuar sozinha");
    } else {
        Texto("Estou sem energia para ajudar agora.", j.x + 96, j.y + 102, 27, COR_MARINHO);
        BotaoEscolha(1, "Continuar sozinha");
    }
}

static void DesenharDicaIA(void)
{
    JanelaMSN();
    Rectangle j = JANELA_MSN;
    Texto("Revelei duas posições para você:", j.x + 96, j.y + 68, 28, COR_MARINHO);
    TextoForte("1ª posição: Y        3ª posição: K", j.x + 96, j.y + 116, 28, COR_ROSA);
    BotaoEscolha(1, "Continuar");
}

void Senha_Desenhar(void)
{
    CarregarTexturas();
    Fundo();

    if (etapa == 1) DesenharEtapa1();
    else DesenharEtapa2();

    if (estado == ETAPA_ACERTOU || estado == ETAPA_FIM) {
        DrawRectangle(0, 0, 1280, 720, Fade(XP_AZUL_ESC, .85f));
        CentroForte("bem-vinda", 640, 300, 64, WHITE);
        Centro("Acesso liberado.", 640, 390, 24, Fade(WHITE, .9f));
    }

    if (estado == ETAPA_POPUP_IA) DesenharPopupIA();
    if (estado == ETAPA_DICA_IA) DesenharDicaIA();
}

bool Senha_Concluida(void)
{
    return estado == ETAPA_FIM;
}