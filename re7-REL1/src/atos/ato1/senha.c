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
static const Rectangle SETA_ETAPA2 = {1062, 240, 42, 42};

static Rectangle RetanguloOpcao(int i)
{
    return (Rectangle){742.0f + (i % 4) * 92.0f, 334.0f + (i / 4) * 70.0f, 78.0f, 56.0f};
}

static Rectangle RetanguloPosicao(int pos)
{
    return (Rectangle){742.0f + pos * 78.0f, 232.0f, 66.0f, 58.0f};
}

static Rectangle RetanguloPeca(int i)
{
    return (Rectangle){742.0f + i * 92.0f, 364.0f, 78.0f, 56.0f};
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
    DrawCircleGradient((Vector2){260, 170}, 520, Fade(XP_AZUL_CLA, .9f), Fade(XP_AZUL, 0));

    /* faixas de cima e de baixo */
    DrawRectangle(0, 0, 1280, 86, XP_AZUL_ESC);
    DrawRectangleGradientH(0, 86, 640, 3, Fade(WHITE, 0), WHITE);
    DrawRectangleGradientH(640, 86, 640, 3, WHITE, Fade(WHITE, 0));
    DrawRectangle(0, 634, 1280, 86, XP_AZUL_ESC);
    DrawRectangleGradientH(0, 631, 640, 3, Fade(XP_LARANJA, 0), XP_LARANJA);
    DrawRectangleGradientH(640, 631, 640, 3, XP_LARANJA, Fade(XP_LARANJA, 0));

    /* separador vertical entre as pistas e o perfil */
    DrawRectangleGradientV(560, 130, 2, 230, Fade(WHITE, 0), Fade(WHITE, .8f));
    DrawRectangleGradientV(560, 360, 2, 230, Fade(WHITE, .8f), Fade(WHITE, 0));

    TextoForte("PC da", 40, 28, 26, WHITE);
    TextoForte("LIA", 40 + UiMeasureTextEmphasis("PC da ", 26), 28, 26, COR_ROSA_TXT);

    /* rodape: botao de desligar no estilo do XP (so decorativo, nao faz nada) e frase */
    Rectangle desligar = {40, 655, 40, 40};
    Rectangle dentro = {desligar.x + 2, desligar.y + 2, 36, 36};
    DrawRectangleRounded(desligar, .28f, 8, WHITE);
    DrawRectangleRounded(dentro, .25f, 8, (Color){180, 45, 20, 255});
    DrawRectangleRounded((Rectangle){dentro.x, dentro.y, dentro.width, dentro.height * 0.55f},
                         .35f, 8, (Color){235, 95, 50, 255});
    DrawRectangleRounded((Rectangle){dentro.x + 4, dentro.y + 3, dentro.width - 8, 9},
                         .6f, 8, Fade(WHITE, .30f));                       /* reflexo de vidro */
    Vector2 meio = {desligar.x + 20, desligar.y + 21};
    DrawRing(meio, 8, 11, -60, 240, 32, WHITE);                           /* abertura em cima */
    DrawRectangleRounded((Rectangle){meio.x - 1.5f, meio.y - 13, 3, 11}, .8f, 4, WHITE);
    Texto("Desligar o computador", 94, 664, 20, WHITE);
    Texto("Depois de entrar, a IA estará esperando por você.", 800, 668, 16, Fade(WHITE, .85f));
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
    Texto(subtitulo, 238, 198, 18, Fade(WHITE, .9f));

    for (int i = 0; i < quantidade; i++) {
        Rectangle r = {90, 290.0f + i * 76.0f, 420, 66};
        DrawRectangleRounded(r, .18f, 8, Fade(XP_AZUL_ESC, .45f));
        DrawRectangleRoundedLinesEx(r, .18f, 8, 1.2f, Fade(WHITE, .5f));
        Texto(linhas[2 * i], r.x + 16, r.y + 10, 15, COR_ROSA_TXT);
        Texto(linhas[2 * i + 1], r.x + 16, r.y + 30, 20, WHITE);
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
    Texto("Digite sua senha:", 742, 206, 18, Fade(WHITE, .9f));
}

static void SetaVerde(Rectangle b)
{
    bool sobre = CheckCollisionPointRec(GetMousePosition(), b);
    DrawRectangleRounded(b, .25f, 6, sobre ? (Color){80, 200, 80, 255} : (Color){60, 170, 60, 255});
    DrawRectangleRoundedLinesEx(b, .25f, 6, 2, WHITE);
    DrawTriangle((Vector2){b.x + 30, b.y + 21}, (Vector2){b.x + 16, b.y + 10},
                 (Vector2){b.x + 16, b.y + 32}, WHITE);
    DrawRectangle((int)b.x + 9, (int)b.y + 17, 10, 8, WHITE);
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

    /* Comeca o arraste: de uma peca da bandeja ou de uma posicao ja preenchida. */
    if (!arrastando && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(mouse, RetanguloPeca(i)) &&
                !CaractereJaUsadoNaEtapa2(PECAS_ORDEM[i])) {
                arrastando = true;
                arrasteCaractere = PECAS_ORDEM[i];
                arrasteOrigem = -1;
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
    Rectangle r = {742, 520, 470, 40};
    DrawRectangleRounded(r, .3f, 8, (Color){255, 255, 225, 255});         /* balao amarelo do XP */
    DrawRectangleRoundedLinesEx(r, .3f, 8, 1.2f, (Color){120, 100, 40, 255});
    Texto(msg, r.x + 14, r.y + 10, 17, COR_MARINHO);
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

    Texto("Escolha quatro caracteres:", 742, 300, 18, Fade(WHITE, .9f));
    for (int i = 0; i < 8; i++)
        BotaoCaractere(RetanguloOpcao(i), OPCOES_FRAGMENTOS[i],
                       CaractereJaUsadoNaEtapa1(OPCOES_FRAGMENTOS[i]), false);

    Texto("Etapa 1 de 2", 742, 486, 16, Fade(WHITE, .75f));
    Aviso();
}

static void DesenharEtapa2(void)
{
    const char *pistas[] = {
        "PISTA 1", "Y aparece antes de K.",
        "PISTA 2", "2 aparece antes de 7.",
        "PISTA 3", "K fica imediatamente depois de 2."
    };
    LadoPistas("Agora organize os caracteres na ordem certa.", pistas, 3);
    Perfil();

    /* o campo de senha vira quatro posicoes */
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 4; i++) {
        Rectangle s = RetanguloPosicao(i);
        bool alvo = arrastando && !posicaoRevelada[i] && CheckCollisionPointRec(mouse, s);
        Color fundo = posicaoRevelada[i] ? (Color){214, 240, 214, 255} :
                      alvo ? (Color){255, 225, 246, 255} : WHITE;
        Color borda = posicaoRevelada[i] ? COR_VERDE : alvo ? COR_ROSA : (Color){60, 90, 180, 255};
        DrawRectangleRounded(s, .14f, 6, fundo);
        DrawRectangleRoundedLinesEx(s, .14f, 6, alvo ? 3.0f : 1.5f, borda);

        char t[2] = {ordem[i] ? ordem[i] : '?', '\0'};
        CentroForte(t, s.x + s.width / 2, s.y + 12, 30,
                    ordem[i] ? COR_MARINHO : (Color){150, 170, 220, 255});

        if (posicaoRevelada[i])
            CentroForte("IA", s.x + s.width / 2, s.y + 62, 15, (Color){170, 240, 170, 255});
        else
            CentroForte(TextFormat("%d", i + 1), s.x + s.width / 2, s.y + 62, 15, Fade(WHITE, .8f));
    }
    SetaVerde(SETA_ETAPA2);

    Texto("Arraste os caracteres para as posições.", 742, 330, 18, Fade(WHITE, .9f));
    for (int i = 0; i < 4; i++)
        BotaoCaractere(RetanguloPeca(i), PECAS_ORDEM[i], false,
                       CaractereJaUsadoNaEtapa2(PECAS_ORDEM[i]));
    Texto("Para tirar um caractere, arraste-o para fora.", 742, 432, 16, Fade(WHITE, .75f));
    Texto("Etapa 2 de 2", 742, 458, 16, Fade(WHITE, .75f));

    if (mostrarDicaIA) {
        Rectangle caixa = {90, 530, 420, 62};
        DrawRectangleRounded(caixa, .2f, 8, Fade(XP_AZUL_ESC, .55f));
        DrawRectangleRoundedLinesEx(caixa, .2f, 8, 1.5f, (Color){170, 240, 170, 255});
        if (texIA.id != 0)
            DesenharTextura(texIA, (Rectangle){0, 0, (float)texIA.width, (float)texIA.height},
                            (Rectangle){caixa.x + 10, caixa.y + 7, 48, 48});
        TextoForte("Ajuda da IA", caixa.x + 68, caixa.y + 8, 15, (Color){170, 240, 170, 255});
        Texto("1ª posição: Y      3ª posição: K", caixa.x + 68, caixa.y + 30, 18, WHITE);
    }

    Aviso();

    /* a peca arrastada vai por cima de tudo, presa ao mouse */
    if (arrastando) {
        Rectangle r = {mouse.x - 33, mouse.y - 29, 66, 58};
        DrawRectangleRounded((Rectangle){r.x + 4, r.y + 6, r.width, r.height},
                             .16f, 6, Fade(BLACK, .3f));
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
    Texto("IA diz:", j.x + 44, j.y + 10, 22, WHITE);
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
    int largura = UiMeasureText(rotulo, 24);
    Texto(rotulo, r.x + (r.width - largura) / 2, r.y + 18, 24, WHITE);
}

static void DesenharPopupIA(void)
{
    JanelaMSN();
    Rectangle j = JANELA_MSN;
    /* SENHA_ERROS_RODADA vale 3. Se mudar, ajuste o texto abaixo. */
    Texto("Você errou três vezes.", j.x + 96, j.y + 68, 24, COR_MARINHO);
    if (Tokens_Valor() > 0) {
        Texto("Quer uma ajuda?", j.x + 96, j.y + 98, 24, COR_MARINHO);
        BotaoEscolha(0, "1   Usar a ajuda");
        BotaoEscolha(1, "2   Continuar sozinha");
    } else {
        Texto("Estou sem energia para ajudar agora.", j.x + 96, j.y + 98, 24, COR_MARINHO);
        BotaoEscolha(1, "1   Continuar sozinha");
    }
}

static void DesenharDicaIA(void)
{
    JanelaMSN();
    Rectangle j = JANELA_MSN;
    Texto("Revelei duas posições para você:", j.x + 96, j.y + 68, 24, COR_MARINHO);
    TextoForte("1ª posição: Y        3ª posição: K", j.x + 96, j.y + 112, 24, COR_ROSA);
    BotaoEscolha(1, "1   Continuar");
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
