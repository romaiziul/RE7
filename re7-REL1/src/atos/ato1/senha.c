// Minijogo da senha do PC da Lia - Ato 1
// Etapa 1: preencher os caracteres de cada fragmento.
// Etapa 2: organizar os caracteres usando as pistas de posicionamento.
// A IA so oferece ajuda na etapa 2, depois de cinco erros.

#include "senha.h"
#include "tokens.h"
#include "raylib.h"
#include <string.h>

#define COR_CLARA    (Color){232, 240, 250, 255}
#define COR_AZUL_MED (Color){143, 178, 213, 255}
#define COR_MARINHO  (Color){29, 38, 96, 255}
#define COR_ROSA     (Color){227, 93, 215, 255}
#define COR_AZUL     (Color){39, 172, 234, 255}
#define COR_VERMELHO (Color){234, 84, 119, 255}
#define COR_VERDE    (Color){76, 175, 80, 255}
#define COR_CIANO    (Color){49, 220, 235, 255}

typedef enum {
    ETAPA_CARACTERES,
    ETAPA_ORDEM,
    ETAPA_POPUP_IA,
    ETAPA_DICA_IA,
    ETAPA_ACERTOU,
    ETAPA_FIM
} EstadoSenha;

static const char SENHA[SENHA_TAMANHO + 1] = "Y2K7";

/* A ordem visual dos fragmentos na etapa 1 e A, C, B, D. */
static const char FRAGMENTOS[] = {'A', 'B', 'C', 'D'};
static const char *PISTAS[] = {
    "Inicial de Kelly Key",
    "Inicial da palavra Year",
    "Ultimo algarismo de 2007",
    "Primeiro algarismo de 2007"
};

/* Caracteres corretos correspondentes aos fragmentos A, C, B e D. */
static const char RESPOSTAS_FRAGMENTOS[] = {'K', 'Y', '7', '2'};

/* Oito opcoes da etapa 1: quatro corretas e quatro distratoras. */
static const char OPCOES_FRAGMENTOS[] = {'A', '7', 'M', 'Y', '2', '9', 'K', '0'};

/* Na etapa 2, as pecas aparecem na ordem solicitada: K, Y, 7, 2. */
static const char PECAS_ORDEM[] = {'K', 'Y', '7', '2'};

static EstadoSenha estado;
static int etapa;
static char caracteresFragmentos[4];
static int opcaoFragmentoSelecionada = -1;
static char ordem[4];
static int pecaSelecionada = -1;
static bool posicaoRevelada[4];
static int erros;
static bool ajudaUsada;
static bool mostrarDicaIA;
static float tempoAviso;
static float tempoAcerto;

static const Rectangle POPUP_IA = {330, 220, 620, 260};
static const Rectangle BOTAO_ACEITAR_IA = {375, 390, 250, 48};
static const Rectangle BOTAO_RECUSAR_IA = {655, 390, 250, 48};
static const Rectangle BOTAO_CONTINUAR_DICA = {490, 475, 300, 50};

static void Centro(const char *texto, float centroX, float y, int tamanho, Color cor)
{
    DrawText(texto,
             (int)(centroX - MeasureText(texto, tamanho) / 2.0f),
             (int)y, tamanho, cor);
}

static bool Clique(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void Fundo(void)
{
    DrawRectangleGradientV(0, 96, 1280, 544,
                           (Color){31, 65, 112, 255},
                           (Color){13, 27, 64, 255});
    DrawRectangle(0, 0, 1280, 96, COR_MARINHO);
    DrawRectangle(0, 640, 1280, 80, COR_MARINHO);
    DrawRectangleGradientH(0, 96, 640, 3, Fade(COR_CLARA, .15f), COR_ROSA);
    DrawRectangleGradientH(640, 96, 640, 3, COR_ROSA, Fade(COR_CLARA, .15f));

    DrawText("PC da", 38, 30, 25, COR_CLARA);
    DrawText("LIA", 125, 30, 25, COR_ROSA);
    DrawText("DESAFIO DE ACESSO", 38, 62, 13, COR_AZUL_MED);
}

static void Painel(Rectangle r, const char *titulo)
{
    DrawRectangleRounded(r, .08f, 8, Fade(COR_MARINHO, .82f));
    DrawRectangleRoundedLinesEx(r, .08f, 8, 1.5f, COR_AZUL_MED);
    DrawText(titulo, (int)r.x + 16, (int)r.y + 9, 15, COR_AZUL_MED);
}

static void BotaoCaractere(Rectangle r, char caractere, bool habilitado,
                           bool selecionado, bool usado)
{
    Color fundo = usado ? Fade(COR_MARINHO, .72f) :
                  selecionado ? COR_ROSA :
                  habilitado ? COR_CLARA : Fade(COR_AZUL_MED, .35f);
    Color texto = (usado || (!habilitado && !selecionado)) ? COR_AZUL_MED :
                  selecionado ? WHITE : COR_MARINHO;

    DrawRectangleRounded(r, .16f, 8, fundo);
    DrawRectangleRoundedLinesEx(r, .16f, 8, 2,
                                selecionado ? COR_ROSA : COR_AZUL);
    char s[2] = {caractere, '\0'};
    Centro(s, r.x + r.width / 2, r.y + (r.height - 28) / 2, 28, texto);
}

static void LimparFragmentos(void)
{
    memset(caracteresFragmentos, 0, sizeof(caracteresFragmentos));
    opcaoFragmentoSelecionada = -1;
}

static void LimparOrdem(void)
{
    for (int i = 0; i < 4; i++) {
        if (!posicaoRevelada[i]) ordem[i] = '\0';
    }
    pecaSelecionada = -1;
}

static bool CaractereJaUsadoNaEtapa1(char c)
{
    for (int i = 0; i < 4; i++)
        if (caracteresFragmentos[i] == c) return true;
    return false;
}

static bool CaractereJaUsadoNaEtapa2(char c)
{
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

static bool TodosFragmentosPreenchidos(void)
{
    for (int i = 0; i < 4; i++)
        if (!caracteresFragmentos[i]) return false;
    return true;
}

static bool OrdemPreenchida(void)
{
    for (int i = 0; i < 4; i++)
        if (!ordem[i]) return false;
    return true;
}

static void RegistrarErroEtapa1(void)
{
    LimparFragmentos();
    tempoAviso = 2.0f;
    /* A etapa 1 nao tem contador de erros nem oferta de ajuda da IA. */
}

static void ConfirmarFragmentos(void)
{
    if (!TodosFragmentosPreenchidos()) {
        tempoAviso = 1.5f;
        return;
    }

    if (FragmentosCorretos()) {
        etapa = 2;
        estado = ETAPA_ORDEM;
        erros = 0;
        tempoAviso = 0.0f;
        LimparOrdem();
    } else {
        RegistrarErroEtapa1();
    }
}

static void RegistrarErroEtapa2(void)
{
    tempoAviso = 1.6f;

    /* Depois da ajuda, os erros deixam de ser contabilizados. */
    if (!ajudaUsada) {
        erros++;
        LimparOrdem();

        if (erros >= SENHA_ERROS_RODADA)
            estado = ETAPA_POPUP_IA;
    } else {
        /* Mantem as posicoes reveladas e nao incrementa o contador. */
        LimparOrdem();
    }
}

static void ConfirmarOrdem(void)
{
    if (!OrdemPreenchida()) {
        tempoAviso = 1.5f;
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

    pecaSelecionada = -1;
    estado = ETAPA_DICA_IA;
}

static void RecusarAjudaIA(void)
{
    /* Recusar nao consome nem recupera tokens; inicia outro ciclo de 5 erros. */
    erros = 0;
    estado = ETAPA_ORDEM;
    tempoAviso = 2.0f;
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
    tempoAcerto = 0.0f;
    memset(posicaoRevelada, 0, sizeof(posicaoRevelada));
    memset(ordem, 0, sizeof(ordem));
    LimparFragmentos();
    pecaSelecionada = -1;
}

/* ------------------------- ATUALIZACAO ------------------------- */

static void AtualizarEtapa1(void)
{
    /* O jogador marca diretamente quatro dos oito caracteres disponiveis. */
    for (int i = 0; i < 8; i++) {
        Rectangle r = {
            235.0f + (i % 4) * 205.0f,
            365.0f + (i / 4) * 62.0f,
            150.0f, 48.0f
        };

        if (Clique(r)) {
            char c = OPCOES_FRAGMENTOS[i];
            int encontrado = -1;

            for (int j = 0; j < 4; j++) {
                if (caracteresFragmentos[j] == c) {
                    encontrado = j;
                    break;
                }
            }

            if (encontrado >= 0) {
                /* Clicar novamente desmarca a opcao. */
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
    }

    if (Clique((Rectangle){500, 530, 280, 44}) ||
        IsKeyPressed(KEY_ENTER)) {
        ConfirmarFragmentos();
    }
}

static void AtualizarEtapa2(void)
{
    /* Seleciona uma peca disponivel. */
    for (int i = 0; i < 4; i++) {
        Rectangle r = {510.0f + i * 165.0f, 405.0f, 130.0f, 66.0f};
        if (Clique(r) && !CaractereJaUsadoNaEtapa2(PECAS_ORDEM[i])) {
            pecaSelecionada = i;
            break;
        }
    }

    /* Depois coloca a peca no slot escolhido. */
    for (int pos = 0; pos < 4; pos++) {
        Rectangle slot = {490.0f + pos * 175.0f, 250.0f, 140.0f, 76.0f};
        if (Clique(slot)) {
            if (posicaoRevelada[pos]) break;

            if (pecaSelecionada >= 0) {
                char c = PECAS_ORDEM[pecaSelecionada];
                bool usada = false;
                for (int j = 0; j < 4; j++)
                    if (j != pos && ordem[j] == c) usada = true;

                if (!usada) ordem[pos] = c;
                pecaSelecionada = -1;
            } else {
                ordem[pos] = '\0';
            }
            break;
        }
    }

    if (Clique((Rectangle){700, 525, 280, 48}) ||
        IsKeyPressed(KEY_ENTER)) {
        ConfirmarOrdem();
    }
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
            if (Clique(BOTAO_ACEITAR_IA) || IsKeyPressed(KEY_ONE))
                AceitarAjudaIA();
            else if (Clique(BOTAO_RECUSAR_IA) || IsKeyPressed(KEY_TWO))
                RecusarAjudaIA();
        } else {
            if (Clique((Rectangle){490, 390, 300, 48}) ||
                IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_TWO)) {
                RecusarAjudaIA();
            }
        }
        break;

    case ETAPA_DICA_IA:
        if (Clique(BOTAO_CONTINUAR_DICA) || IsKeyPressed(KEY_ENTER))
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

static void DesenharEtapa1(void)
{
    Centro("ETAPA 1 / 2: DESCUBRA OS CARACTERES", 640, 108, 21, WHITE);
    Centro("Leia as pistas e selecione os quatro caracteres que formam a senha.",
           640, 140, 16, COR_CLARA);

    Painel((Rectangle){100, 165, 1080, 140}, "PISTAS DOS FRAGMENTOS");

    for (int i = 0; i < 4; i++) {
        Rectangle card = {125.0f + i * 265.0f, 200.0f, 245.0f, 88.0f};
        DrawRectangleRounded(card, .12f, 8, COR_CLARA);
        DrawRectangleRoundedLinesEx(card, .12f, 8, 2, COR_AZUL);
        Centro(TextFormat("FRAGMENTO %c", FRAGMENTOS[i]),
               card.x + card.width / 2, card.y + 7, 14, COR_ROSA);

        const char *linha1 = "";
        const char *linha2 = "";
        if (i == 0) { linha1 = "Inicial de"; linha2 = "Kelly Key"; }
        else if (i == 1) { linha1 = "Ultimo algarismo"; linha2 = "de 2007"; }
        else if (i == 2) { linha1 = "Inicial da palavra"; linha2 = "Year"; }
        else { linha1 = "Primeiro algarismo"; linha2 = "de 2007"; }

        Centro(linha1, card.x + card.width / 2, card.y + 31, 17, COR_MARINHO);
        Centro(linha2, card.x + card.width / 2, card.y + 55, 17, COR_MARINHO);
    }

    Painel((Rectangle){190, 325, 900, 165}, "ESCOLHA QUATRO CARACTERES");

    for (int i = 0; i < 8; i++) {
        Rectangle r = {
            235.0f + (i % 4) * 205.0f,
            365.0f + (i / 4) * 62.0f,
            150.0f, 48.0f
        };
        bool selecionado = CaractereJaUsadoNaEtapa1(OPCOES_FRAGMENTOS[i]);
        BotaoCaractere(r, OPCOES_FRAGMENTOS[i], true, selecionado, false);
    }

    int quantidade = QuantidadeSelecionadaEtapa1();
    Centro(TextFormat("%d de 4 selecionados", quantidade),
           640, 500, 15, COR_CLARA);

    Rectangle confirmar = {500, 530, 280, 44};
    DrawRectangleRounded(confirmar, .16f, 8,
                         quantidade == 4 ? COR_AZUL_MED : Fade(COR_AZUL_MED, .65f));
    DrawRectangleRoundedLinesEx(confirmar, .16f, 8, 1.5f, COR_AZUL);
    Centro("CONFIRMAR", 640, 543, 18, WHITE);

    if (tempoAviso > 0.0f) {
        Centro("Os caracteres selecionados nao correspondem aos fragmentos. Tente novamente.",
               640, 595, 14, COR_ROSA);
    }
}

static void DesenharContadorErros(void)
{
    Rectangle painel = {1000, 108, 205, 54};
    DrawRectangleRounded(painel, .12f, 8, Fade(COR_MARINHO, .9f));
    DrawRectangleRoundedLinesEx(painel, .12f, 8, 1.5f,
                                ajudaUsada ? COR_VERDE : COR_ROSA);

    Centro(ajudaUsada ? "ERROS: 0/5" : TextFormat("ERROS: %d/5", erros),
           1102, 113, 14, COR_CLARA);

    for (int i = 0; i < SENHA_ERROS_RODADA; i++) {
        Rectangle slot = {1020.0f + i * 34.0f, 135, 23, 17};
        bool preenchido = !ajudaUsada && i < erros;
        DrawRectangleRounded(slot, .2f, 6,
                             preenchido ? COR_VERMELHO : Fade(COR_AZUL_MED, .18f));
        DrawRectangleRoundedLinesEx(slot, .2f, 6, 1.2f,
                                    preenchido ? COR_ROSA : COR_AZUL);
        if (preenchido)
            Centro("X", slot.x + slot.width / 2, slot.y + 1, 14, WHITE);
    }
}

static void DesenharEtapa2(void)
{
    Centro("ETAPA 2 / 2: DESCUBRA A ORDEM", 640, 108, 21, WHITE);
    Centro("Organize os quatro caracteres na ordem correta para formar a senha.",
           640, 143, 16, COR_CLARA);
    DesenharContadorErros();

    Painel((Rectangle){75, 175, 380, 300}, "PISTAS DE POSICIONAMENTO");
    const char *pistas[] = {
        "1. Y aparece antes de K.",
        "2. 2 aparece antes de 7.",
        "3. K fica imediatamente",
        "   depois de 2.",
        "4. 7 ocupa a ultima",
        "   posicao."
    };
    for (int i = 0; i < 6; i++)
        DrawText(pistas[i], 100, 218 + i * 37, 18, COR_CLARA);

    Painel((Rectangle){470, 175, 735, 165}, "ORGANIZE OS CARACTERES");
    for (int i = 0; i < 4; i++) {
        float x = 490.0f + i * 175.0f;
        Centro(TextFormat("%d", i + 1), x + 70, 218, 19, COR_AZUL_MED);
        Rectangle slot = {x, 250, 140, 76};
        DrawRectangleRounded(slot, .14f, 8,
                             posicaoRevelada[i] ? Fade(COR_VERDE, .28f) : COR_CLARA);
        DrawRectangleRoundedLinesEx(slot, .14f, 8, 2.5f,
                                    posicaoRevelada[i] ? COR_VERDE : COR_AZUL);
        if (ordem[i]) {
            char s[2] = {ordem[i], '\0'};
            Centro(s, x + 70, 265, 40, COR_MARINHO);
        } else {
            Centro("?", x + 70, 265, 38, COR_AZUL_MED);
        }
        if (posicaoRevelada[i])
            Centro("IA", x + 70, 328, 12, COR_VERDE);
    }

    Painel((Rectangle){470, 350, 735, 145}, "CARACTERES DISPONIVEIS");
    for (int i = 0; i < 4; i++) {
        Rectangle r = {510.0f + i * 165.0f, 405, 130, 66};
        bool usado = CaractereJaUsadoNaEtapa2(PECAS_ORDEM[i]);
        BotaoCaractere(r, PECAS_ORDEM[i], !usado,
                       pecaSelecionada == i, usado);
    }

    if (mostrarDicaIA) {
        DrawRectangleRounded((Rectangle){75, 495, 380, 90}, .08f, 8,
                             Fade(COR_MARINHO, .9f));
        DrawRectangleRoundedLinesEx((Rectangle){75, 495, 380, 90},
                                    .08f, 8, 2, COR_VERDE);
        DrawText("AJUDA DA IA", 95, 507, 15, COR_VERDE);
        DrawText("1a posicao: Y", 95, 533, 15, COR_CLARA);
        DrawText("3a posicao: K", 95, 555, 15, COR_CLARA);
    }

    Rectangle confirmar = {700, 525, 280, 48};
    DrawRectangleRounded(confirmar, .16f, 8, COR_AZUL_MED);
    DrawRectangleRoundedLinesEx(confirmar, .16f, 8, 1.5f, COR_AZUL);
    Centro("CONFIRMAR", confirmar.x + confirmar.width / 2,
           confirmar.y + 13, 18, WHITE);

    if (tempoAviso > 0.0f) {
        const char *mensagem = ajudaUsada
            ? "Sequencia incorreta. Revise as pistas e tente novamente."
            : "Sequencia incorreta. Revise as pistas e tente novamente.";
        Centro(mensagem, 835, 590, 14, COR_ROSA);
    }
}

static void DesenharPopupIA(void)
{
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .58f));
    DrawRectangleRounded(POPUP_IA, .06f, 10, (Color){13, 27, 64, 250});
    DrawRectangleRoundedLinesEx(POPUP_IA, .06f, 10, 3, COR_ROSA);
    DrawRectangleRounded((Rectangle){350, 238, 580, 7}, .5f, 8, COR_AZUL);

    Centro("ASSISTENTE DA LIA", 640, 265, 25, COR_CLARA);
    Centro("Voce errou cinco vezes. Quer uma ajuda para continuar?",
           640, 315, 18, COR_CLARA);

    if (Tokens_Valor() > 0) {
        DrawRectangleRounded(BOTAO_ACEITAR_IA, .16f, 8, COR_CIANO);
        Centro("USAR AJUDA (-1 TOKEN)", 500, 403, 17, COR_MARINHO);

        DrawRectangleRounded(BOTAO_RECUSAR_IA, .16f, 8, COR_AZUL_MED);
        Centro("CONTINUAR SEM AJUDA", 780, 403, 17, COR_MARINHO);
        Centro(TextFormat("Tokens disponiveis: %d/3", Tokens_Valor()),
               640, 460, 15, COR_AZUL_MED);
    } else {
        Centro("A IA esta sem energia no momento.", 640, 360, 17, COR_ROSA);
        DrawRectangleRounded((Rectangle){490, 390, 300, 48}, .16f, 8, COR_AZUL_MED);
        Centro("CONTINUAR SOZINHA", 640, 403, 17, COR_MARINHO);
    }
}

static void DesenharDicaIA(void)
{
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .66f));

    /* Modal centralizado, com espacamento uniforme entre titulo, dicas e acao. */
    Rectangle painel = {310, 165, 660, 395};
    DrawRectangleRounded(painel, .06f, 10, (Color){13, 27, 64, 252});
    DrawRectangleRoundedLinesEx(painel, .06f, 10, 3, COR_ROSA);

    DrawCircle(390, 235, 29, COR_CIANO);
    Centro("IA", 390, 221, 25, COR_MARINHO);
    DrawText("ASSISTENTE DA LIA", 435, 217, 26, COR_CLARA);
    DrawText("Usei uma carga para revelar duas posicoes:", 385, 282, 18, COR_CLARA);

    Rectangle dica1 = {385, 330, 510, 54};
    Rectangle dica2 = {385, 400, 510, 54};
    DrawRectangleRounded(dica1, .12f, 8, Fade(COR_AZUL, .28f));
    DrawRectangleRounded(dica2, .12f, 8, Fade(COR_AZUL, .28f));
    DrawRectangleRoundedLinesEx(dica1, .12f, 8, 1.2f, COR_AZUL);
    DrawRectangleRoundedLinesEx(dica2, .12f, 8, 1.2f, COR_AZUL);

    Centro("1a posicao: Y", 640, 345, 21, COR_CIANO);
    Centro("3a posicao: K", 640, 415, 21, COR_CIANO);

    DrawRectangleRounded(BOTAO_CONTINUAR_DICA, .16f, 8, COR_CIANO);
    Centro("CONTINUAR", 640, 490, 19, COR_MARINHO);
}

void Senha_Desenhar(void)
{
    Fundo();

    if (etapa == 1) DesenharEtapa1();
    else DesenharEtapa2();

    if (estado == ETAPA_ACERTOU || estado == ETAPA_FIM) {
        DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .62f));
        Rectangle painel = {250, 245, 780, 220};
        DrawRectangleRounded(painel, .08f, 10, (Color){13, 27, 64, 245});
        DrawRectangleRoundedLinesEx(painel, .08f, 10, 3, COR_AZUL);
        DrawRectangleRounded((Rectangle){270, 265, 740, 8}, .5f, 8, COR_ROSA);
        Centro("ACESSO LIBERADO!", 640, 315, 40, COR_VERDE);
        Centro("A senha foi descoberta.", 640, 380, 22, COR_CLARA);
        Centro("O computador esta desbloqueado.", 640, 415, 20, WHITE);
    }

    if (estado == ETAPA_POPUP_IA) DesenharPopupIA();
    if (estado == ETAPA_DICA_IA) DesenharDicaIA();

    Centro("ESC: sair do desafio", 640, 685, 15, COR_AZUL_MED);
}

bool Senha_Concluida(void)
{
    return estado == ETAPA_FIM;
}
