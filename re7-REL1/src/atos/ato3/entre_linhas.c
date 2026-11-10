// entre_linhas.c
// Tela do Desafio 3 "ENTRE AS LINHAS" (quadro 1280x720, como o resto do jogo).
// Fundo: o auditorio escurecido. No centro, o papel do discurso; embaixo, as
// palavras numa janela no estilo do MSN. Ao errar, a Lia gagueja e o amigo
// sopra a palavra da plateia (caixa de dialogo do jogo).
#include "entre_linhas.h"

#include "raylib.h"
#include "ui.h"
#include "ato3.h"
#include "tokens.h"
#include "audio_manager.h"

#include <math.h>
#include <stddef.h>

static EntreLinhas desafio;
static float tremorErro = 0.0f;         // a tela oscila um instante ao errar

#define TREMOR_ERRO_SEGUNDOS  0.45f
#define REVELAR_SEGUNDOS      0.5f      // a palavra ganha nitidez

#define COR_ROSA      (Color){255, 105, 200, 255}
#define COR_AZUL_MSN  (Color){39, 172, 234, 255}
#define COR_MARINHO   (Color){29, 38, 96, 255}
#define COR_VERDE     (Color){76, 175, 80, 255}

static const Rectangle PAPEL = {290, 104, 700, 250};
static const Rectangle JANELA = {290, 376, 700, 176};


// =====================================================
// INICIO E ATUALIZACAO
// =====================================================

void InitAto3EntreLinhas(Ato3Ajuda modo)
{
    EntreLinhas_Iniciar(&desafio, modo);
    tremorErro = 0.0f;
}

// Botoes: tres colunas na janela. Com ajuda, a terceira coluna mostra a
// opcao que a IA tirou (riscada, sem clique).
static Rectangle AreaBotao(int coluna)
{
    const float margem = 20.0f, folga = 16.0f;
    float largura = (JANELA.width - 2 * margem - 2 * folga) / 3.0f;

    return (Rectangle){
        JANELA.x + margem + coluna * (largura + folga),
        JANELA.y + 60.0f,
        largura,
        72.0f
    };
}

MinigameResult UpdateAto3EntreLinhas(void)
{
    float dt = GetFrameTime();

    EntreLinhas_Atualizar(&desafio, dt);

    if (tremorErro > 0.0f) tremorErro -= dt;

    if (desafio.fase == EL_FASE_ESCOLHENDO)
    {
        const Ato3Etapa *etapa = Ato3_Etapa(desafio.modo, desafio.etapa);

        for (int i = 0; i < etapa->total; i++)
        {
            if (!IsButtonClicked(AreaBotao(i)) && !IsKeyPressed(KEY_ONE + i)) continue;

            EntreLinhasResposta r = EntreLinhas_Responder(&desafio, i);

            if (r == EL_RESPOSTA_CERTA)
            {
                AudioManager_Play(SOM_MOUSE);
                AudioManager_Play(SOM_ACERTO);
            }
            else if (r == EL_RESPOSTA_ERRADA)
            {
                AudioManager_Play(SOM_ERRO);
                tremorErro = TREMOR_ERRO_SEGUNDOS;
            }

            break;      // um clique so responde uma vez
        }
    }
    else if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
             IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (EntreLinhas_Avancar(&desafio))
            AudioManager_Play(SOM_DIALOGO);
    }

    return EntreLinhas_Terminou(&desafio) ? MINIGAME_VENCEU : MINIGAME_CONTINUA;
}


// =====================================================
// DESENHO
// =====================================================

static float Limitar01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// Nitidez de cada palavra, com a animacao de quem acabou de ser recuperada
static void CalcularNitidez(float nitidez[ATO3_ETAPAS])
{
    for (int i = 0; i < ATO3_ETAPAS; i++)
    {
        nitidez[i] = desafio.nitida[i] ? 1.0f : 0.0f;

        bool revelando = (i == desafio.etapa) &&
                         (desafio.fase == EL_FASE_ACERTO || desafio.fase == EL_FASE_ERRO_AMIGO);

        if (revelando) nitidez[i] = Limitar01(desafio.tempoFase / REVELAR_SEGUNDOS);
    }
}

// Personagem pela borda esquerda, mantendo a proporcao da imagem
static void DesenharRetrato(Texture2D tex, float altura, float x, float yTopo, bool espelhar)
{
    float largura = altura * (float)tex.width / (float)tex.height;

    DrawTexturePro(
        tex,
        (Rectangle){0, 0, espelhar ? -(float)tex.width : (float)tex.width, (float)tex.height},
        (Rectangle){x, yTopo, largura, altura},
        (Vector2){0, 0}, 0.0f, WHITE
    );
}

static void DesenharCabecalho(void)
{
    const char *titulo = "ENTRE AS LINHAS";
    int fs = 34;
    UiDrawTextEmphasis(titulo, (1280 - UiMeasureTextEmphasis(titulo, fs)) / 2, 18, fs, WHITE);

    int etapaVisivel = desafio.etapa < ATO3_ETAPAS ? desafio.etapa + 1 : ATO3_ETAPAS;
    const char *sub = (desafio.fase >= EL_FASE_CONCLUSAO)
        ? "Discurso recuperado"
        : TextFormat("Palavra %d de %d", etapaVisivel, ATO3_ETAPAS);

    DrawCenteredText(sub, 60, 20, SKYBLUE);

    // tres bolinhas de progresso
    for (int i = 0; i < ATO3_ETAPAS; i++)
    {
        Vector2 c = {640.0f - 24.0f + i * 24.0f, 92.0f};
        bool feita = desafio.nitida[i];

        if (feita) DrawCircleV(c, 6, desafio.errou[i] ? SKYBLUE : COR_VERDE);
        else DrawCircleLinesV(c, 6, Fade(WHITE, 0.6f));
    }
}

// Visto verde desenhado em linhas (a fonte nao tem o simbolo)
static void DesenharVisto(Vector2 c, float tamanho, Color cor)
{
    DrawLineEx((Vector2){c.x - tamanho, c.y}, (Vector2){c.x - tamanho * 0.3f, c.y + tamanho * 0.7f}, 4.0f, cor);
    DrawLineEx((Vector2){c.x - tamanho * 0.3f, c.y + tamanho * 0.7f}, (Vector2){c.x + tamanho, c.y - tamanho * 0.7f}, 4.0f, cor);
}

static void DesenharJanelaOpcoes(void)
{
    const Ato3Etapa *etapa = Ato3_Etapa(desafio.modo, desafio.etapa);
    bool comAjuda = (desafio.modo == ATO3_AJUDA_TIRAR_OPCAO);

    // janela no estilo do MSN
    DrawRectangleRounded(JANELA, 0.06f, 8, (Color){232, 240, 250, 245});
    DrawRectangleRounded((Rectangle){JANELA.x, JANELA.y, JANELA.width, 40}, 0.4f, 8, COR_AZUL_MSN);
    DrawRectangleRoundedLines(JANELA, 0.06f, 8, COR_MARINHO);

    // status da IA: com a barra em 0 ela ja esta offline (a ajuda foi o ultimo token)
    bool online = !Tokens_IAOffline();
    const char *titulo;

    if (!comAjuda)    titulo = "IA: Entendido. Você está por conta própria.";
    else if (online)  titulo = "IA diz: tirei uma opção errada. A escolha é sua.";
    else              titulo = "IA (offline): a opção errada já foi tirada.";

    DrawCircle((int)JANELA.x + 24, (int)JANELA.y + 20, 8, (comAjuda && online) ? COR_VERDE : GRAY);
    UiDrawText(titulo, (int)JANELA.x + 42, (int)JANELA.y + 9, 20, WHITE);

    Vector2 mouse = GetMousePosition();
    bool escolhendo = (desafio.fase == EL_FASE_ESCOLHENDO);

    for (int i = 0; i < etapa->total; i++)
    {
        Rectangle r = AreaBotao(i);
        bool certa = (desafio.fase == EL_FASE_ACERTO && i == etapa->correta);
        bool sobre = escolhendo && CheckCollisionPointRec(mouse, r);

        Color fundo = certa ? COR_VERDE : (sobre ? COR_ROSA : (Color){29, 38, 96, 235});

        DrawRectangleRounded(r, 0.3f, 8, fundo);
        DrawRectangleRoundedLines(r, 0.3f, 8, WHITE);

        const char *texto = TextFormat("%d   %s", i + 1, etapa->opcoes[i]);
        int larg = UiMeasureText(texto, 26);
        UiDrawText(texto, (int)(r.x + (r.width - larg) / 2), (int)r.y + 22, 26, WHITE);
    }

    // com ajuda: a opcao que a IA tirou aparece riscada
    if (comAjuda && etapa->removida != NULL)
    {
        Rectangle r = AreaBotao(2);
        DrawRectangleRounded(r, 0.3f, 8, Fade(GRAY, 0.35f));

        int larg = UiMeasureText(etapa->removida, 24);
        int tx = (int)(r.x + (r.width - larg) / 2);
        UiDrawText(etapa->removida, tx, (int)r.y + 14, 24, Fade(DARKGRAY, 0.8f));
        DrawLineEx((Vector2){tx - 6.0f, r.y + 28}, (Vector2){tx + larg + 6.0f, r.y + 28}, 3.0f, (Color){200, 60, 90, 255});

        const char *nota = "removida pela IA";
        UiDrawText(nota, (int)(r.x + (r.width - UiMeasureText(nota, 16)) / 2), (int)r.y + 46, 16, DARKGRAY);
    }

    if (escolhendo)
    {
        const char *dica = (etapa->total == 3) ? "Clique na palavra ou use as teclas 1, 2 e 3"
                                               : "Clique na palavra ou use as teclas 1 e 2";
        UiDrawText(dica, (int)(JANELA.x + (JANELA.width - UiMeasureText(dica, 17)) / 2),
                   (int)(JANELA.y + JANELA.height - 30), 17, COR_MARINHO);
    }
}

static void DesenharConclusao(void)
{
    float t = desafio.tempoFase;
    float entrada = Limitar01(t / 0.6f);

    // brilho dourado em volta do papel
    for (int i = 0; i < 3; i++)
    {
        float cresce = 6.0f + i * 6.0f + 4.0f * sinf((float)GetTime() * 3.0f);
        DrawRectangleRoundedLines(
            (Rectangle){PAPEL.x - cresce, PAPEL.y - cresce, PAPEL.width + 2 * cresce, PAPEL.height + 2 * cresce},
            0.04f, 8, Fade((Color){255, 214, 120, 255}, (0.45f - i * 0.13f) * entrada)
        );
    }

    // faixa de conclusao
    Rectangle faixa = {440, 386, 400, 64};
    DrawRectangleRounded(faixa, 0.5f, 8, Fade(COR_MARINHO, 0.92f * entrada));
    DrawRectangleRoundedLines(faixa, 0.5f, 8, Fade((Color){255, 214, 120, 255}, entrada));

    const char *texto = "Discurso recuperado!";
    UiDrawTextEmphasis(texto, (int)(faixa.x + (faixa.width - UiMeasureTextEmphasis(texto, 28)) / 2),
                       (int)faixa.y + 16, 28, Fade(WHITE, entrada));

    if (t >= EL_TEMPO_CONCLUSAO)
        DrawCenteredText("CLIQUE ou ESPAÇO para continuar", 474, 20, Fade(WHITE, 0.85f));
}

void DrawAto3EntreLinhas(Assets *assets)
{
    float agora = (float)GetTime();
    int recuperadas = EntreLinhas_Recuperadas(&desafio);
    bool concluiu = desafio.fase >= EL_FASE_CONCLUSAO;

    // auditorio ao fundo, escurecido
    Ato3Cena cena = {
        .pcOnline = !Tokens_IAOffline(),
        .pcPiscando = false,
        .aplausos = concluiu,
        .professoraSorri = concluiu,
        .foco = 0.0f,
        .nervosismo = (float)(ATO3_ETAPAS - recuperadas) / ATO3_ETAPAS
    };
    Ato3_DesenharCenario(&cena);
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, concluiu ? 0.35f : 0.55f));

    DesenharCabecalho();

    // nervosismo: o papel treme menos a cada palavra recuperada
    float nervoso = 1.2f * (float)(ATO3_ETAPAS - recuperadas) / ATO3_ETAPAS;
    Vector2 tremor = {sinf(agora * 31.0f) * nervoso, cosf(agora * 27.0f) * nervoso * 0.5f};

    if (tremorErro > 0.0f)
        tremor.x += sinf(agora * 70.0f) * 7.0f * (tremorErro / TREMOR_ERRO_SEGUNDOS);

    float nitidez[ATO3_ETAPAS];
    CalcularNitidez(nitidez);

    int destaque = (desafio.fase == EL_FASE_ESCOLHENDO || desafio.fase == EL_FASE_ERRO_LIA)
                   ? desafio.etapa : -1;

    // Lia a direita (a expressao acompanha o desafio)
    Texture2D lia = assets->lia07[9];                       // pensativa
    if (desafio.fase == EL_FASE_ACERTO) lia = assets->lia07[7];          // feliz
    else if (desafio.fase == EL_FASE_ERRO_LIA) lia = assets->lia07[1];   // confusa
    else if (desafio.fase == EL_FASE_ERRO_AMIGO) lia = assets->lia07[3]; // surpresa
    else if (concluiu) lia = assets->lia07[10];                         // determinada

    DesenharRetrato(lia, 440.0f, 980.0f, 280.0f, false);

    // o amigo aparece a esquerda quando sopra a palavra
    if (desafio.fase == EL_FASE_ERRO_AMIGO)
        DesenharRetrato(assets->amigo[0], 440.0f, -10.0f, 280.0f, false);

    Ato3_DesenharPapel(PAPEL, 27, nitidez, destaque, tremor);

    const Ato3Etapa *etapa = Ato3_Etapa(desafio.modo, desafio.etapa);

    switch (desafio.fase)
    {
        case EL_FASE_ESCOLHENDO:
            DesenharJanelaOpcoes();
            break;

        case EL_FASE_ACERTO:
            DesenharJanelaOpcoes();
            DesenharVisto((Vector2){PAPEL.x + PAPEL.width - 40, PAPEL.y + 34}, 14.0f,
                          Fade(COR_VERDE, 1.0f - Limitar01(desafio.tempoFase / EL_TEMPO_ACERTO) * 0.5f));
            break;

        case EL_FASE_ERRO_LIA:
            DrawDialogueBox("Lia", etapa->falaErroLia);
            break;

        case EL_FASE_ERRO_AMIGO:
            DrawDialogueBox("João", etapa->falaAmigo);
            break;

        case EL_FASE_CONCLUSAO:
        case EL_FASE_FIM:
            DesenharConclusao();
            break;
    }
}


// =====================================================
// MODO DESENVOLVEDOR
// =====================================================

bool Ato3EntreLinhas_DevPular(void)
{
    switch (desafio.fase)
    {
        case EL_FASE_ACERTO:
            EntreLinhas_Atualizar(&desafio, EL_TEMPO_ACERTO);
            return true;

        case EL_FASE_ERRO_LIA:
        case EL_FASE_ERRO_AMIGO:
            desafio.bloqueio = 0.0f;
            return EntreLinhas_Avancar(&desafio);

        case EL_FASE_CONCLUSAO:
            desafio.bloqueio = 0.0f;
            desafio.tempoFase = EL_TEMPO_CONCLUSAO;
            return EntreLinhas_Avancar(&desafio);

        default:
            return false;       // ha uma escolha na tela
    }
}

void Ato3EntreLinhas_DevResolverEtapa(void)
{
    if (desafio.fase == EL_FASE_ESCOLHENDO)
    {
        desafio.bloqueio = 0.0f;
        EntreLinhas_Responder(&desafio, Ato3_Etapa(desafio.modo, desafio.etapa)->correta);
        EntreLinhas_Atualizar(&desafio, EL_TEMPO_ACERTO);
        return;
    }

    Ato3EntreLinhas_DevPular();
}
