#include "game.h"

#include "ui.h"
#include "scenes.h"
#include "minigame.h"
#include "ato2_corrida.h"
#include "story_ato1.h"
#include "story_ato2.h"
#include "senha.h"
#include "tokens.h"
#include "barra_tokens.h"
#include "room.h"
#include "audio_manager.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ato3.h"
#include "ato3_logica.h"
#include "entre_linhas.h"
#include "story_ato3.h"


// =====================================================
// FUNDO COSMICO DO MENU
// =====================================================

static void BrilhoSuave(float centroX, float centroY, float raio, Color cor, float opacidade)
{
    DrawCircleGradient((Vector2){centroX, centroY}, raio, Fade(cor, opacidade), Fade(cor, 0.0f));
}

static void DesenharFundo(void)
{
    int larguraTela = GetScreenWidth();
    int alturaTela = GetScreenHeight();
    float tempo = (float)GetTime();

    // base: violeta-noite para azul-marinho
    DrawRectangleGradientV(0, 0, larguraTela, alturaTela, (Color){24, 14, 52, 255}, (Color){12, 34, 66, 255});

    // manchas de luz nas cores do logo, flutuando devagar
    BrilhoSuave(larguraTela * 0.20f + sinf(tempo * 0.30f) * 30, alturaTela * 0.30f, 340, (Color){255, 105, 200, 255}, 0.35f);
    BrilhoSuave(larguraTela * 0.80f + cosf(tempo * 0.25f) * 30, alturaTela * 0.70f, 380, (Color){ 80, 200, 255, 255}, 0.30f);
    BrilhoSuave(larguraTela * 0.50f,                        alturaTela * 0.40f, 300, (Color){190, 150, 255, 255}, 0.25f);

    // estrelinhas piscando
    for (int i = 0; i < 45; i++)
    {
        unsigned int semente = (unsigned int)(i + 1) * 2654435761u;
        float posX = (float)(semente % (unsigned)larguraTela);
        float posY = (float)((semente >> 8) % (unsigned)alturaTela);
        float tamanho = 3.0f + (float)((semente >> 16) % 5);
        float brilho = 0.35f + 0.65f * (0.5f + 0.5f * sinf(tempo * 2.0f + i));
        Color corEstrela = Fade(WHITE, brilho);

        DrawLineEx((Vector2){posX - tamanho, posY}, (Vector2){posX + tamanho, posY}, 1.5f, corEstrela);
        DrawLineEx((Vector2){posX, posY - tamanho}, (Vector2){posX, posY + tamanho}, 1.5f, corEstrela);
        DrawCircleV((Vector2){posX, posY}, 1.5f, corEstrela);
    }
}


// =====================================================
// SAVE / LOAD (arquivo de texto: "cena fala tokens reserva iaEscreveu")
// =====================================================

// O build de desenvolvimento usa outro arquivo: os testes nunca mexem no
// progresso salvo da jogadora.
#ifdef RE7_DEV_MODE
#define SAVE_FILE "save_dev.dat"
#else
#define SAVE_FILE "save.dat"
#endif

static bool SaveGame(GameState *game)
{
    return SaveFileText(
        SAVE_FILE,
        (char *)TextFormat(
            "%d %d %d %d %d",
            (int)game->currentScene,
            game->dialogueIndex,
            Tokens_Valor(),
            Tokens_ReservaUsada() ? 1 : 0,
            game->iaEscreveuDiscurso ? 1 : 0
        )
    );
}

// Retorna true se conseguiu carregar um save valido
static bool LoadGame(GameState *game)
{
    if (!FileExists(SAVE_FILE)) return false;

    char *text = LoadFileText(SAVE_FILE);

    if (text == NULL) return false;

    int scene = 0;
    int index = 0;
    int tokens = TOKENS_MAX;

    int reserva = 0;
    int iaEscreveu = 0;
    int lidos = sscanf(text, "%d %d %d %d %d", &scene, &index, &tokens, &reserva, &iaEscreveu);

    UnloadFileText(text);

    // Saves antigos tem tres ou quatro campos e ainda podem ser carregados.
    if (lidos < 3 || lidos > 5) return false;

    if (scene < 0 || scene >= SCENE_COUNT) return false;

    if (tokens < 0 || tokens > TOKENS_MAX) tokens = TOKENS_MAX;

    Scene s = GetScene((SceneID)scene);

    if (index < 0 || index >= s.dialogueCount) index = 0;

    game->currentScene = (SceneID)scene;
    game->dialogueIndex = index;
    game->choice = 0;
    game->iaEscreveuDiscurso = (iaEscreveu == 1);
    game->campanha = CAMPANHA_EM_ANDAMENTO;

    Tokens_Restaurar(tokens, reserva == 1);
    BarraTokens_Reiniciar();

    return true;
}


// =====================================================
// EFEITO DE VIAGEM NO TEMPO (flash) E PERSONAGEM
// =====================================================

#define FLASH_DURACAO 1.4f

// Fade para preto (transicao lenta entre cenas)
typedef enum
{
    FADE_NENHUM,
    FADE_ESCURECENDO,       // a tela vai ficando preta; no fim troca de cena
    FADE_SEGURANDO,         // tela preta com um texto (ex.: "No outro dia...")
    FADE_CLAREANDO          // a tela volta a aparecer
} FadeFase;

#define FADE_SAIDA_ENCONTRO     1.8f    // escurece depois da fala "Ufa, consegui alcançá-lo!"
#define FADE_TRANSICAO          1.2f    // escurece entre uma cena e outra do Ato 2
#define FADE_ENTRADA_LANHOUSE   1.27f    // a nova cena clareia
#define FADE_ENTRADA_ENCONTRO   0.45f   // o encontro aparece depois da corrida
#define FADE_TEXTO_SEGUNDOS     3.0f    // quanto tempo o texto fica na tela preta
#define ENCONTRO_VIRA_SEGUNDOS  1.1f    // quanto tempo o amigo demora para virar e olhar para a Lia

static FadeFase fadeFase = FADE_NENHUM;
static float fadeTempo = 0.0f;
static float fadeDuracao = 1.0f;
static SceneID fadeDestino = SCENE_PRESENT;
static float encontroTempo = 0.0f;          // tempo desde que o encontro comecou

static void IniciarFade(FadeFase fase, float duracao, SceneID destino)
{
    fadeFase = fase;
    fadeTempo = 0.0f;
    fadeDuracao = duracao;
    fadeDestino = destino;
}

// Texto mostrado na tela preta antes de entrar numa cena (NULL = nenhum)
static const char *TextoDeTransicao(SceneID destino)
{
    if (destino == SCENE_ATO2_CG_FRENTE_ESCOLA) return "No outro dia...";
    return NULL;
}

// Cenas que seguram a tela preta antes de aparecer (com ou sem texto)
static bool SegurarPretoAntes(SceneID destino)
{
    return destino == SCENE_ATO2_CG_FRENTE_ESCOLA;
}

static void DesenharFade(void)
{
    if (fadeFase == FADE_NENHUM) return;

    float t = fadeTempo / fadeDuracao;
    if (t > 1.0f) t = 1.0f;

    float alfa = 1.0f;
    if (fadeFase == FADE_ESCURECENDO) alfa = t;
    else if (fadeFase == FADE_CLAREANDO) alfa = 1.0f - t;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alfa));

    // o texto entra e sai com o mesmo fade da tela
    const char *texto = TextoDeTransicao(fadeDestino);
    if (texto != NULL)
    {
        int tamanho = 44;
        int largura = MeasureText(texto, tamanho);
        DrawText(texto, (GetScreenWidth() - largura) / 2, GetScreenHeight() / 2 - tamanho / 2,
                 tamanho, Fade(WHITE, alfa));
    }
}

// =====================================================
// SEQUENCIA DO COMPUTADOR (fim do Ato 1 -> Ato 2)
// Boot do "Doors XP" -> area de trabalho -> o mouse clica no navegador
// "Orquidea" -> pagina da rede social, com o mouse acompanhando a leitura.
// Tudo desenhado em codigo (o cursor tambem). Posicoes em fracao da tela (0 a 1).
// =====================================================

static bool  pcAtivo = false;
static float pcTempo = 0.0f;
static float pcSaida = -1.0f;      // < 0: ainda na sequencia; >= 0: escurecendo para sair

#define PC_BOOT_ENTRA        0.7f     // a tela de boot aparece saindo do preto
#define PC_BOOT_FIM          4.7f     // fim do boot
#define PC_DESK_FADE         0.6f     // boot -> area de trabalho
#define PC_CLIQUE_1          7.9f     // duplo clique no icone
#define PC_CLIQUE_2          8.2f
#define PC_PERFIL_INICIO     8.5f     // area de trabalho -> pagina da rede social
#define PC_PERFIL_FADE       0.4f
#define PC_ESPERA           12.0f     // o mouse terminou o titulo; a pagina fica ate o jogador apertar ESPACO
#define PC_SAIDA             1.0f     // fade para preto no fim

// Icone do navegador "Windows Explosion" na area de trabalho (retangulo do icone + legenda)
static const Rectangle PC_ICONE = { 0.015f, 0.180f, 0.092f, 0.198f };
// Trilho da barra de carregamento do boot (parte de dentro)
static const Rectangle PC_TRILHO = { 0.386f, 0.7545f, 0.228f, 0.0148f };

typedef struct { float t; float x; float y; bool suave; } PcPonto;

// Caminho do mouse (tempo em segundos desde o inicio da sequencia)
static const PcPonto pcCaminho[] =
{
    {  5.3f, 0.82f, 0.60f, false },   // aparece
    {  7.3f, 0.058f, 0.250f, true },  // vai ate o icone
    {  8.9f, 0.058f, 0.250f, false }, // clica duas vezes e espera a pagina abrir
    { 10.2f, 0.250f, 0.100f, true },  // vai ate o titulo da comunidade
    { 11.6f, 0.480f, 0.100f, false }, // acompanha o titulo
    { 12.0f, 0.480f, 0.100f, false },
};

static Vector2 PcPosicaoMouse(float t)
{
    int n = (int)(sizeof(pcCaminho) / sizeof(pcCaminho[0]));
    float larg = (float)GetScreenWidth();
    float alt = (float)GetScreenHeight();

    if (t <= pcCaminho[0].t) return (Vector2){ pcCaminho[0].x * larg, pcCaminho[0].y * alt };
    if (t >= pcCaminho[n - 1].t) return (Vector2){ pcCaminho[n - 1].x * larg, pcCaminho[n - 1].y * alt };

    for (int i = 1; i < n; i++)
    {
        if (t <= pcCaminho[i].t)
        {
            float u = (t - pcCaminho[i - 1].t) / (pcCaminho[i].t - pcCaminho[i - 1].t);
            if (pcCaminho[i].suave) u = u * u * (3.0f - 2.0f * u);   // acelera e freia
            float x = pcCaminho[i - 1].x + (pcCaminho[i].x - pcCaminho[i - 1].x) * u;
            float y = pcCaminho[i - 1].y + (pcCaminho[i].y - pcCaminho[i - 1].y) * u;
            return (Vector2){ x * larg, y * alt };
        }
    }
    return (Vector2){ 0, 0 };
}

// Seta de mouse (contorno preto, miolo branco)
static void PcDesenharCursor(Vector2 p, bool apertado)
{
    float e = apertado ? 0.92f : 1.0f;
    Vector2 poli[7] = { {0, 0}, {0, 17}, {4, 13}, {7, 20}, {10, 19}, {7, 12}, {12, 12} };
    for (int i = 0; i < 7; i++) { poli[i].x = p.x + poli[i].x * 1.5f * e; poli[i].y = p.y + poli[i].y * 1.5f * e; }

    // miolo: dois pedacos convexos (desenhados nas duas orientacoes)
    Vector2 a[3] = { poli[0], poli[1], poli[6] };
    Vector2 b[4] = { poli[2], poli[3], poli[4], poli[5] };
    DrawTriangle(a[0], a[1], a[2], WHITE); DrawTriangle(a[0], a[2], a[1], WHITE);
    DrawTriangle(b[0], b[1], b[2], WHITE); DrawTriangle(b[0], b[2], b[1], WHITE);
    DrawTriangle(b[0], b[2], b[3], WHITE); DrawTriangle(b[0], b[3], b[2], WHITE);
    DrawTriangle(poli[1], poli[2], poli[6], WHITE); DrawTriangle(poli[1], poli[6], poli[2], WHITE);

    for (int i = 0; i < 7; i++)
        DrawLineEx(poli[i], poli[(i + 1) % 7], 2.0f, BLACK);
}

static void PcDesenharImagem(Texture2D tex, float alfa)
{
    DrawTexturePro(
        tex,
        (Rectangle){ 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
        (Vector2){ 0, 0 },
        0.0f,
        Fade(WHITE, alfa)
    );
}

// Tres blocos azuis que correm no trilho, como na tela de boot
static void PcDesenharBarra(float t)
{
    float larg = (float)GetScreenWidth();
    float alt = (float)GetScreenHeight();
    float x0 = PC_TRILHO.x * larg, y0 = PC_TRILHO.y * alt;
    float w = PC_TRILHO.width * larg, h = PC_TRILHO.height * alt;

    float bloco = h * 0.9f;
    float passo = bloco * 1.35f;
    float ciclo = w + 3.0f * passo;
    float pos = fmodf(t * 150.0f, ciclo) - 3.0f * passo;       // 150 px por segundo

    BeginScissorMode((int)x0, (int)y0, (int)w, (int)h);
    for (int i = 0; i < 3; i++)
    {
        float bx = x0 + pos + i * passo;
        DrawRectangleGradientV((int)bx, (int)(y0 + h * 0.05f), (int)bloco, (int)(h * 0.9f),
                               (Color){150, 190, 255, 255}, (Color){30, 80, 210, 255});
    }
    EndScissorMode();
}

static void PcDesenhar(const Assets *assets)
{
    float t = pcTempo;
    float larg = (float)GetScreenWidth();
    float alt = (float)GetScreenHeight();

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

    if (t < PC_BOOT_FIM + PC_DESK_FADE)
    {
        float a = (t < PC_BOOT_ENTRA) ? t / PC_BOOT_ENTRA : 1.0f;
        PcDesenharImagem(assets->pcBoot, a);
        if (t >= PC_BOOT_ENTRA * 0.5f) PcDesenharBarra(t);
    }

    if (t >= PC_BOOT_FIM)
    {
        float a = (t - PC_BOOT_FIM) / PC_DESK_FADE;
        if (a > 1.0f) a = 1.0f;
        PcDesenharImagem(assets->pcDesktop, a);

        // icone selecionado depois do primeiro clique
        if (t >= PC_CLIQUE_1 && t < PC_PERFIL_INICIO + PC_PERFIL_FADE)
        {
            Rectangle r = { PC_ICONE.x * larg, PC_ICONE.y * alt, PC_ICONE.width * larg, PC_ICONE.height * alt };
            DrawRectangleRounded(r, 0.08f, 6, Fade((Color){49, 106, 197, 255}, 0.35f));
            DrawRectangleLinesEx(r, 1.5f, Fade((Color){120, 170, 255, 255}, 0.9f));
        }
    }

    if (t >= PC_PERFIL_INICIO)
    {
        float a = (t - PC_PERFIL_INICIO) / PC_PERFIL_FADE;
        if (a > 1.0f) a = 1.0f;
        PcDesenharImagem(assets->pcPerfil, a);
    }

    // mouse
    if (t >= pcCaminho[0].t)
    {
        Vector2 m = PcPosicaoMouse(t);
        bool apertado = false;
        float cliques[2] = { PC_CLIQUE_1, PC_CLIQUE_2 };
        for (int i = 0; i < 2; i++)
        {
            float d = t - cliques[i];
            if (d >= 0.0f && d < 0.12f) apertado = true;
            if (d >= 0.0f && d < 0.35f)
                DrawCircleLinesV(m, 6.0f + d * 70.0f, Fade(WHITE, 1.0f - d / 0.35f));
        }
        PcDesenharCursor(m, apertado);
    }

    // saida: fade para preto
    if (pcSaida >= 0.0f)
    {
        float a = pcSaida / PC_SAIDA;
        if (a > 1.0f) a = 1.0f;
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, a));
    }
    else if (t >= PC_ESPERA)
    {
        DrawText("ESPAÇO  >>", 1040, 672, 22, Fade(WHITE, 0.85f));
    }
}

// Tempo restante do flash (0 = sem flash)
static float flashTimer = 0.0f;

// Tempo restante da mensagem "opcao invalida" na tela final
static float avisoInvalido = 0.0f;

// Controles do painel de audio
static Rectangle audioFechar = {1040, 80, 100, 45};

static Rectangle audioTestar = {470, 430, 220, 55};
static Rectangle audioMudo = {470, 500, 220, 55};
static Rectangle sliderMusica = {400, 270, 400, 12};
static Rectangle sliderEfeitos = {400, 360, 400, 12};

// Tempo restante do aviso "som ligado" (F2)
static float avisoSom = 0.0f;

// Desenha um controle deslizante de volume
static void DesenharSlider(Rectangle r, float valor, const char *titulo)
{
    UiDrawText(titulo, (int)r.x, (int)r.y - 32, 22, WHITE);

    DrawRectangleRec(r, GRAY);

    DrawRectangleRec(
        (Rectangle){r.x, r.y, r.width * valor, r.height},
        SKYBLUE
    );

    DrawCircle(
        (int)(r.x + r.width * valor),
        (int)(r.y + r.height / 2),
        10,
        WHITE
    );
}

// Aviso de som no canto inferior direito
static void DesenharAvisoSom(void)
{
    bool mutado = AudioManager_IsMuted();

    if (!mutado && avisoSom <= 0.0f) return;

    const char *texto = mutado ? "Som desligado (F2)" : "Som ligado";

    int larguraTexto = UiMeasureText(texto, 18);

    Rectangle caixa = {
        (float)GetScreenWidth() - larguraTexto - 44.0f,
        (float)GetScreenHeight() - 40.0f,
        larguraTexto + 28.0f,
        30.0f
    };

    DrawRectangleRounded(caixa, 0.4f, 8, Fade(BLACK, 0.65f));

    UiDrawText(
        texto,
        (int)caixa.x + 14,
        (int)caixa.y + 6,
        18,
        mutado ? (Color){ 255, 105, 200, 255 } : SKYBLUE
    );
}

// Desenha uma personagem mantendo a proporcao da imagem.
// altura: altura na tela | yTopo: posicao vertical | xDireita: borda direita
// espelhar: true vira a imagem na horizontal (largura negativa na origem)
static void DrawPersonagemEx(Texture2D tex, float altura, float yTopo, float xDireita, bool espelhar)
{
    float largura = altura * (float)tex.width / (float)tex.height;

    Rectangle destino = { xDireita - largura, yTopo, largura, altura };

    DrawTexturePro(
        tex,
        (Rectangle){ 0, 0, espelhar ? -(float)tex.width : (float)tex.width, (float)tex.height },
        destino,
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

static void DrawPersonagem(Texture2D tex, float altura, float yTopo, float xDireita)
{
    DrawPersonagemEx(tex, altura, yTopo, xDireita, false);
}

// Igual ao DrawPersonagemEx, mas a posicao horizontal e a da borda ESQUERDA
// (as medidas do Figma: X, Y e altura)
static void DrawPersonagemEsq(Texture2D tex, float altura, float yTopo, float xEsquerda, bool espelhar)
{
    float largura = altura * (float)tex.width / (float)tex.height;

    DrawPersonagemEx(tex, altura, yTopo, xEsquerda + largura, espelhar);
}

// Posicao e tamanho dos personagens no Ato 2 (medidas do Figma, quadro 1280x720)
#define ATO2_JOAO_X      110.0f
#define ATO2_JOAO_Y      -22.0f
#define ATO2_JOAO_ALTURA 884.0f
#define ATO2_LIA_X       712.0f
#define ATO2_LIA_Y        25.0f
#define ATO2_LIA_ALTURA  735.0f

// Flash branco-rosado com "glitch" que se apaga aos poucos
static void DesenharFlash(void)
{
    if (flashTimer <= 0.0f) return;

    float intensidade = flashTimer / FLASH_DURACAO;      // 1 -> 0

    DrawRectangle(
        0, 0, GetScreenWidth(), GetScreenHeight(),
        Fade((Color){255, 235, 250, 255}, intensidade)
    );

    // listras de glitch no comeco do efeito
    if (intensidade > 0.55f)
    {
        for (int i = 0; i < 10; i++)
        {
            int posY = GetRandomValue(0, GetScreenHeight() - 20);
            int alturaListra = GetRandomValue(4, 18);
            int posX = GetRandomValue(0, 300);

            Color corListra = (i % 2 == 0) ? (Color){255, 105, 200, 255} : (Color){80, 200, 255, 255};

            DrawRectangle(posX, posY, GetScreenWidth() - posX, alturaListra, Fade(corListra, 0.5f * intensidade));
        }
    }
}

// Avatar da IA: a estrela do logo, flutuando ao lado da caixa de dialogo
static void DesenharAvatarIA(Assets *assets)
{
    float bob = sinf((float)GetTime() * 3.0f) * 5.0f;

    float cx = 200.0f;
    float cy = 470.0f + bob;

    DrawCircleGradient(
        (Vector2){cx, cy}, 95.0f,
        Fade((Color){255, 105, 200, 255}, 0.45f),
        Fade((Color){255, 105, 200, 255}, 0.0f)
    );

    DrawTexturePro(
        assets->iaStar,
        (Rectangle){ 0, 0, (float)assets->iaStar.width, (float)assets->iaStar.height },
        (Rectangle){ cx - 60.0f, cy - 60.0f, 120.0f, 120.0f },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}


// =====================================================
// MUSICA DE FUNDO
// =====================================================

static void AtualizarMusica(GameState *game)
{
    switch (game->currentScreen)
    {
        case SCREEN_MENU:
        case SCREEN_CREDITS:
            AudioManager_PlayMusic(MUSICA_RE7);
            break;

        case SCREEN_ENDING:
            AudioManager_PlayMusic(MUSICA_FINAL);
            break;

        case SCREEN_ROOM:
        case SCREEN_SENHA:
            AudioManager_PlayMusic(MUSICA_2007);
            break;

        case SCREEN_CORRIDA:
            // A musica da corrida e controlada em ato2_corrida.c.
            break;

        case SCREEN_ENTRE_LINHAS:
            AudioManager_PlayMusic(MUSICA_2007);
            break;

        case SCREEN_GAME:
        {
            Scene scene = GetScene(game->currentScene);
            bool ultimaFala =
                game->dialogueIndex >= scene.dialogueCount - 1;

            switch (game->currentScene)
            {
                case SCENE_PRESENT:
                    AudioManager_PlayMusic(
                        ultimaFala ? MUSICA_VIAGEM : MUSICA_RE7
                    );
                    break;

                case SCENE_ATO2_CG_ESCREVEM:
                case SCENE_ATO2_CG_PROFESSORA:
                case SCENE_ATO2_CG_PROFESSORA_SEM_IA:
                case SCENE_ATO2_CG_CASA_NOITE:
                case SCENE_ATO2_CG_FRENTE_ESCOLA:
                case SCENE_ATO2_CG_ENSAIO:
                    AudioManager_PlayMusic(MUSICA_VITORIA);
                    break;

                case SCENE_ATO3_CONCLUSAO:
                case SCENE_ATO3_CONCLUSAO_IA_ESCREVEU:
                case SCENE_ATO3_ETAPA_CONCLUIDA:
                    AudioManager_PlayMusic(MUSICA_VITORIA);
                    break;

                case SCENE_EPILOGUE:
                    AudioManager_PlayMusic(MUSICA_EPILOGO);
                    break;

                default:
                    // Ato 1 e Ato 2
                    AudioManager_PlayMusic(MUSICA_2007);
                    break;
            }
            break;
        }

        default:
            break;
    }
}


// =====================================================
// VISUAL DE CADA CENA (fundo e personagens)
// =====================================================

typedef enum
{
    FUNDO_2026,
    FUNDO_2007,
    FUNDO_CORREDOR,
    FUNDO_LANHOUSE,
    FUNDO_CG_TELA,
    FUNDO_CG_ESCREVEM,
    FUNDO_CG_PROFESSORA,
    FUNDO_CG_ENSAIO,
    FUNDO_CG_CASA_NOITE,        // Lia treinando em casa, de noite
    FUNDO_CG_FRENTE_ESCOLA      // frente da escola, de dia
} FundoId;

typedef struct
{
    FundoId fundo;
    int lia;            // 0 = nao aparece, 2026 ou 2007
    bool amigo;         // true = o amigo aparece a esquerda
} VisualCena;

static VisualCena VisualDaCena(SceneID id)
{
    switch (id)
    {
        case SCENE_PRESENT:             return (VisualCena){ FUNDO_CG_TELA,        0,    false };

        case SCENE_2007:
        case SCENE_2007_PC:
        case SCENE_ATO1_IA_A:
        case SCENE_ATO1_IA_B:
        case SCENE_ATO2_QUARTO:         return (VisualCena){ FUNDO_2007,           2007, false };

        // corredor: o Joao aparece na cena 2 e no resultado 2.1 (nao no 2.2)
        case SCENE_ATO2_CORREDOR:
        case SCENE_ATO2_DETERMINADA:
        case SCENE_ATO2_ENCONTRO:
        case SCENE_ATO2_RES21:          return (VisualCena){ FUNDO_CORREDOR,       2007, true  };

        case SCENE_ATO2_RES22:          return (VisualCena){ FUNDO_CORREDOR,       2007, false };

        // lan house: o Joao esta ao lado da Lia (na cena 3.2 ele entra durante a cena)
        case SCENE_ATO2_LANHOUSE:
        case SCENE_ATO2_LANHOUSE_SEM:
        case SCENE_ATO2_IA_ESCREVE:
        case SCENE_ATO2_UMA_IDEIA:
        case SCENE_ATO2_COM_JOAO:
        case SCENE_ATO2_C4:
        case SCENE_ATO2_C4_ZERO:        return (VisualCena){ FUNDO_LANHOUSE,       2007, true  };

        case SCENE_ATO2_CG_CASA_NOITE:  return (VisualCena){ FUNDO_CG_CASA_NOITE,     0, false };
        case SCENE_ATO2_CG_FRENTE_ESCOLA: return (VisualCena){ FUNDO_CG_FRENTE_ESCOLA,  0, false };

        case SCENE_ATO2_CG_ESCREVEM:    return (VisualCena){ FUNDO_CG_ESCREVEM,    0,    false };
        case SCENE_ATO2_CG_PROFESSORA:
        case SCENE_ATO2_CG_PROFESSORA_SEM_IA:
                                        return (VisualCena){ FUNDO_CG_PROFESSORA,  0,    false };
        case SCENE_ATO2_CG_ENSAIO:      return (VisualCena){ FUNDO_CG_ENSAIO,      0,    false };

        case SCENE_EPILOGUE:            return (VisualCena){ FUNDO_2026,           2026, false };

        default:                        return (VisualCena){ FUNDO_2026,           2026, false };
    }
}

// Expressao da ultima fala (ate a fala atual) de quem esta em "quem"
static Pose PoseDe(const Scene *scene, int indice, const char *quem)
{
    for (int i = indice; i >= 0; i--)
    {
        if (strcmp(scene->dialogues[i].speaker, quem) == 0)
            return scene->dialogues[i].pose;
    }

    return POSE_PADRAO;
}

static Texture2D TexturaLia(Assets *assets, int era, Pose pose)
{
    if (era == 2026)
    {
        switch (pose)
        {
            case POSE_PREOCUPADA: return assets->lia26[1];
            case POSE_DECIDIDA:   return assets->lia26[2];
            default:              return assets->lia26[0];
        }
    }

    switch (pose)
    {
        case POSE_CONFUSA:      return assets->lia07[1];
        case POSE_BRAVA:        return assets->lia07[2];
        case POSE_SURPRESA:     return assets->lia07[3];
        case POSE_CONTRARIADA:  return assets->lia07[4];
        case POSE_NOSTALGICA:   return assets->lia07[5];
        case POSE_RINDO:        return assets->lia07[6];
        case POSE_FELIZ:        return assets->lia07[7];
        case POSE_TRISTE:       return assets->lia07[8];
        case POSE_PENSATIVA:    return assets->lia07[9];
        case POSE_DETERMINADA:  return assets->lia07[10];
        default:                return assets->lia07[0];
    }
}

// Preenche a tela inteira com uma imagem
static void DesenharImagemTela(Texture2D tex)
{
    DrawTexturePro(
        tex,
        (Rectangle){ 0, 0, (float)tex.width, (float)tex.height },
        (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

// Zoom sutil e lento em CG: a imagem cresce ate ~6% em 10 s, mantendo o
// ponto de foco (fx, fy, de 0 a 1) parado na tela. O tempo reinicia quando
// a cena muda (nao a cada fala).
static void DesenharImagemZoom(Texture2D tex, SceneID cena, float fx, float fy)
{
    static SceneID cenaAnterior = (SceneID)-1;
    static float tempo = 0.0f;
    static double ultimoDesenho = -1.0;
    double agora = GetTime();
    // reinicia se a cena mudou ou se a CG ficou um tempo sem ser desenhada
    if (cena != cenaAnterior || agora - ultimoDesenho > 0.3) { cenaAnterior = cena; tempo = 0.0f; }
    ultimoDesenho = agora;
    if (fadeFase != FADE_SEGURANDO) tempo += GetFrameTime();   // nao conta o tempo da tela preta

    float t = tempo / 10.0f;
    if (t > 1.0f) t = 1.0f;
    t = t * (2.0f - t);                      // suaviza a chegada
    float s = 1.0f + 0.06f * t;

    float w = (float)tex.width / s;
    float h = (float)tex.height / s;
    float x0 = fx * ((float)tex.width - w);
    float y0 = fy * ((float)tex.height - h);

    DrawTexturePro(
        tex,
        (Rectangle){ x0, y0, w, h },
        (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

// =====================================================
// ESCOLHAS (aparecem sobre a propria tela de dialogo)
// =====================================================

typedef enum
{
    ESCOLHA_NENHUMA = 0,
    ESCOLHA_ANO,        // Ato 1: em que ano voce esta?
    ESCOLHA_AMIGO,      // Ato 2: dar uma chance ao amigo
    ESCOLHA_IA,         // Ato 2: o que responder a IA na lan house
    ESCOLHA_PAPEL       // Ato 3: ajuda da IA com o papel do discurso
} EscolhaId;

typedef struct
{
    const char *pergunta;
    int total;
    const char *opcoes[3];
} Escolha;

static const Escolha ESCOLHAS[] =
{
    [ESCOLHA_ANO] =
    {
        ATO1_ESCOLHA_ANO_PERGUNTA, 3,
        { ATO1_ESCOLHA_ANO_A, ATO1_ESCOLHA_ANO_B, ATO1_ESCOLHA_ANO_C }
    },

    [ESCOLHA_AMIGO] =
    {
        ATO2_ESCOLHA1_PERGUNTA, 2,
        { ATO2_ESCOLHA1_A, ATO2_ESCOLHA1_B, "" }
    },

    [ESCOLHA_IA] =
    {
        ATO2_ESCOLHA2_PERGUNTA, 3,
        { ATO2_ESCOLHA2_A, ATO2_ESCOLHA2_B, ATO2_ESCOLHA2_C }
    },

    // a ordem segue Ato3Ajuda (ato3_logica.h)
    [ESCOLHA_PAPEL] =
    {
        ATO3_ESCOLHA_PERGUNTA, 3,
        { ATO3_ESCOLHA_A, ATO3_ESCOLHA_B, ATO3_ESCOLHA_C }
    }
};

// Escolhas que aparecem como janela do MSN
static bool EscolhaNoMSN(int escolha)
{
    return escolha == ESCOLHA_IA || escolha == ESCOLHA_PAPEL;
}

// Opcao errada: pisca em vermelho por um instante e a escolha continua
static float escolhaErro = 0.0f;
static int escolhaErroOpcao = -1;

// Area clicavel de cada opcao
static Rectangle AreaOpcao(int escolha, int indice)
{
    float topo = EscolhaNoMSN(escolha) ? 250.0f : 230.0f;

    return (Rectangle){ 340.0f, topo + indice * 84.0f, 600.0f, 66.0f };
}

// Janela de MSN (so na escolha da IA)
static void DesenharPopupMSN(void)
{
    Rectangle janela = { 300.0f, 110.0f, 680.0f, 420.0f };

    DrawRectangleRounded(janela, 0.04f, 8, (Color){ 232, 240, 250, 245 });
    DrawRectangleRounded((Rectangle){ janela.x, janela.y, janela.width, 44.0f }, 0.3f, 8, (Color){ 39, 172, 234, 255 });
    DrawRectangleRoundedLines(janela, 0.04f, 8, (Color){ 29, 38, 96, 255 });

    DrawCircle((int)janela.x + 26, (int)janela.y + 22, 8, (Color){ 76, 175, 80, 255 });
    UiDrawText("IA diz:", (int)janela.x + 44, (int)janela.y + 11, 22, WHITE);
}

static void DesenharEscolha(GameState *game)
{
    const Escolha *e = &ESCOLHAS[game->choice];

    // escurece o fundo
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.45f));

    if (EscolhaNoMSN(game->choice))
    {
        DesenharPopupMSN();
        UiDrawText(e->pergunta, 340, 175, 32, (Color){ 29, 38, 96, 255 });
    }
    else
    {
        DrawCenteredText(e->pergunta, 160, 36, WHITE);
    }

    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < e->total; i++)
    {
        Rectangle r = AreaOpcao(game->choice, i);
        bool sobre = CheckCollisionPointRec(mouse, r);
        bool erro = (escolhaErro > 0.0f && escolhaErroOpcao == i);

        Color fundo = erro ? (Color){ 200, 60, 90, 255 }
                           : (sobre ? (Color){ 255, 105, 200, 255 } : (Color){ 29, 38, 96, 235 });

        DrawRectangleRounded(r, 0.3f, 8, fundo);
        DrawRectangleRoundedLines(r, 0.3f, 8, WHITE);

        const char *texto = e->opcoes[i];
        int larguraTexto = UiMeasureText(texto, 30);

        UiDrawText(texto, (int)(r.x + (r.width - larguraTexto) / 2.0f), (int)r.y + 18, 30, WHITE);
    }
}

// Cena 4: barra em 0 (a IA escreveu) ou em 1 a 3. Vale com ou sem a corrida.
static SceneID ProximaCena4(void)
{
    return (Tokens_Valor() == 0) ? SCENE_ATO2_C4_ZERO : SCENE_ATO2_C4;
}

// Aplica o efeito da opcao escolhida
static void ResolverEscolha(GameState *game, int opcao)
{
    switch (game->choice)
    {
        case ESCOLHA_ANO:
            if (opcao == 2)                         // 2007: certo
            {
                game->choice = ESCOLHA_NENHUMA;
                ChangeScene(game, SCENE_ATO1_IA_B);
            }
            else                                    // 2005 ou 2006: a escolha volta
            {
                escolhaErro = 0.5f;
                escolhaErroOpcao = opcao;
                return;
            }
            break;

        case ESCOLHA_AMIGO:
            game->choice = ESCOLHA_NENHUMA;

            if (opcao == 0)                         // tentar alcancar o Joao: corrida
            {
                IniciarFade(FADE_ESCURECENDO, FADE_TRANSICAO, SCENE_ATO2_DETERMINADA);
            }
            else                                    // nao preciso de amigos: pula a corrida
            {
                IniciarFade(FADE_ESCURECENDO, FADE_TRANSICAO, SCENE_ATO2_RES22);
            }
            break;

        case ESCOLHA_IA:
            game->choice = ESCOLHA_NENHUMA;

            if (opcao == 0)                         // IA escreve o discurso: zera
            {
                Tokens_Zerar();
                game->iaEscreveuDiscurso = true;    // muda falas do Ato 3
                ChangeScene(game, SCENE_ATO2_IA_ESCREVE);
            }
            else if (opcao == 1)                    // so uma ideia: -1
            {
                Tokens_Gastar();
                ChangeScene(game, SCENE_ATO2_UMA_IDEIA);
            }
            else                                    // fazer com a ajuda do Joao: +1
            {
                Tokens_Recuperar();
                ChangeScene(game, SCENE_ATO2_COM_JOAO);
            }
            break;

        case ESCOLHA_PAPEL:
        {
            game->choice = ESCOLHA_NENHUMA;

            Ato3Ajuda ajuda = (Ato3Ajuda)opcao;     // 0 IA le, 1 tirar opcao, 2 sozinha
            Ato3_AplicarAjuda(ajuda);
            ChangeScene(game, Ato3_CenaDaAjuda(ajuda, game->iaEscreveuDiscurso));
            break;
        }

        default:
            break;
    }

    SaveGame(game);
}

static void AtualizarEscolha(GameState *game)
{
    if (escolhaErro > 0.0f) escolhaErro -= GetFrameTime();

    const Escolha *e = &ESCOLHAS[game->choice];

    for (int i = 0; i < e->total; i++)
    {
        if (IsButtonClicked(AreaOpcao(game->choice, i)) || IsKeyPressed(KEY_ONE + i))
        {
            AudioManager_Play(SOM_MOUSE);
            ResolverEscolha(game, i);
            return;
        }
    }
}


// =====================================================
// FIM DA CAMPANHA (epilogo ou final ruim)
// =====================================================

static void EncerrarCampanha(GameState *game, EstadoCampanha final, MotivoFinalRuim motivo)
{
    game->campanha = final;
    game->motivoFinalRuim = motivo;
    game->choice = ESCOLHA_NENHUMA;

    fadeFase = FADE_NENHUM;
    pcAtivo = false;

    // fim da historia: o save volta ao inicio (para jogar de novo)
    // e a tela final aparece
    ChangeScene(game, SCENE_PRESENT);
    SaveGame(game);
    game->currentScreen = SCREEN_ENDING;
}


// =====================================================
// FIM DE CADA CENA: o que acontece depois da ultima fala
// =====================================================

static void FimDaCena(GameState *game, Scene scene)
{
    switch (game->currentScene)
    {
        case SCENE_2007:
            // Fim da chegada em 2007: explorar o quarto.
            game->dialogueIndex = scene.dialogueCount - 1;
            SaveGame(game);
            InitRoom();
            game->currentScreen = SCREEN_ROOM;
            return;

        case SCENE_2007_PC:
            // PC bloqueado: minijogo da senha.
            game->dialogueIndex = scene.dialogueCount - 1;
            SaveGame(game);
            Senha_Iniciar();
            game->senhaNoQuarto = false;
            game->currentScreen = SCREEN_SENHA;
            return;

        case SCENE_ATO1_IA_A:
            game->dialogueIndex = scene.dialogueCount - 1;
            game->choice = ESCOLHA_ANO;
            escolhaErro = 0.0f;
            return;

        case SCENE_ATO2_CORREDOR:
            game->dialogueIndex = scene.dialogueCount - 1;
            game->choice = ESCOLHA_AMIGO;
            escolhaErro = 0.0f;
            return;

        case SCENE_ATO2_DETERMINADA:
            // Desafio 2: corrida de obstaculos.
            game->dialogueIndex = scene.dialogueCount - 1;
            SaveGame(game);
            InitAto2Corrida();
            game->currentScreen = SCREEN_CORRIDA;
            return;

        case SCENE_ATO2_ENCONTRO:
            // depois da fala: a tela escurece devagar e o corredor clareia sozinho
            game->dialogueIndex = scene.dialogueCount - 1;
            IniciarFade(FADE_ESCURECENDO, FADE_SAIDA_ENCONTRO, scene.nextScene);
            return;

        case SCENE_ATO2_LANHOUSE:
        case SCENE_ATO2_LANHOUSE_SEM:
            // escolha 2 (pop-up do MSN)
            game->dialogueIndex = scene.dialogueCount - 1;
            game->choice = ESCOLHA_IA;
            escolhaErro = 0.0f;
            return;

        case SCENE_ATO2_IA_ESCREVE:
        case SCENE_ATO2_UMA_IDEIA:
        case SCENE_ATO2_COM_JOAO:
            // cena 4: a versao depende da escolha 1 (Joao) e da barra (0 ou 1 a 3)
            game->dialogueIndex = scene.dialogueCount - 1;
            IniciarFade(FADE_ESCURECENDO, FADE_TRANSICAO, ProximaCena4());
            return;

        // transicoes com fade para preto entre uma cena e outra
        case SCENE_ATO2_QUARTO:
        case SCENE_ATO2_RES21:
        case SCENE_ATO2_RES22:
        case SCENE_ATO2_C4:
        case SCENE_ATO2_C4_ZERO:
        case SCENE_ATO2_CG_ESCREVEM:
        case SCENE_ATO2_CG_CASA_NOITE:
        case SCENE_ATO2_CG_ENSAIO:
        case SCENE_ATO1_IA_B:           // fim do Ato 1: cartao "FIM DO ATO 1" e entra no Ato 2
            game->dialogueIndex = scene.dialogueCount - 1;
            IniciarFade(FADE_ESCURECENDO, FADE_TRANSICAO, scene.nextScene);
            return;

        case SCENE_ATO2_CG_FRENTE_ESCOLA:
            // barra em 0: a professora sem a IA; barra em 1 a 3: com "Etapa concluida"
            game->dialogueIndex = scene.dialogueCount - 1;
            IniciarFade(
                FADE_ESCURECENDO, FADE_TRANSICAO,
                (Tokens_Valor() == 0) ? SCENE_ATO2_CG_PROFESSORA_SEM_IA : SCENE_ATO2_CG_PROFESSORA
            );
            return;

        // ---------------- Ato 3 ----------------

        case SCENE_ATO2_CG_PROFESSORA:
        case SCENE_ATO2_CG_PROFESSORA_SEM_IA:
            // fim do Ato 2: escurece e abre o auditorio
            game->dialogueIndex = scene.dialogueCount - 1;
            IniciarFade(FADE_ESCURECENDO, FADE_TRANSICAO, scene.nextScene);
            return;

        case SCENE_ATO3_ABERTURA:
            // barra em 0: a IA volta com o token de reserva (uma unica vez)
            ChangeScene(game, Ato3_CenaAposAbertura());
            return;

        case SCENE_ATO3_OFERTA:
            // escolha do papel (pop-up do MSN)
            game->dialogueIndex = scene.dialogueCount - 1;
            game->choice = ESCOLHA_PAPEL;
            escolhaErro = 0.0f;
            return;

        case SCENE_ATO3_AJUDA:
        case SCENE_ATO3_SOZINHA:
            // Desafio 3: "Entre as Linhas"
            game->dialogueIndex = scene.dialogueCount - 1;
            SaveGame(game);
            InitAto3EntreLinhas(game->currentScene == SCENE_ATO3_SOZINHA
                                    ? ATO3_AJUDA_SOZINHA : ATO3_AJUDA_TIRAR_OPCAO);
            game->currentScreen = SCREEN_ENTRE_LINHAS;
            return;

        case SCENE_ATO3_IA_LE:
        case SCENE_ATO3_IA_LE_IA_ESCREVEU:
            // cenario 1: final ruim, qualquer que fosse a barra
            EncerrarCampanha(game, CAMPANHA_FINAL_RUIM, FINAL_RUIM_IA_LEU);
            return;

        case SCENE_ATO3_CONCLUSAO:
        case SCENE_ATO3_CONCLUSAO_IA_ESCREVEU:
            // com pelo menos 1 token: "Etapa concluida" e epilogo; com 0: final ruim
            if (Ato3_TerminaNoEpilogo())
                ChangeScene(game, SCENE_ATO3_ETAPA_CONCLUIDA);
            else
                EncerrarCampanha(game, CAMPANHA_FINAL_RUIM, FINAL_RUIM_SEM_TOKENS);
            return;

        case SCENE_EPILOGUE:
            EncerrarCampanha(game, CAMPANHA_FINAL_BOM, FINAL_RUIM_IA_LEU);
            return;

        default:
            break;
    }

    ChangeScene(game, scene.nextScene);

    // viagem no tempo: flash ao chegar em 2007 ou ao voltar ao presente
    if (game->currentScene == SCENE_2007 || game->currentScene == SCENE_EPILOGUE)
        flashTimer = FLASH_DURACAO;
}


// =====================================================
// POSIÇÕES DOS BOTÕES DO MENU
// =====================================================

#define MENU_BUTTONS 4

static Rectangle menuRects[MENU_BUTTONS] =
{
    {490, 439, 299, 59},   // 0 - Novo Jogo
    {515, 505, 250, 50},   // 1 - Continuar
    {515, 565, 250, 50},   // 2 - Creditos
    {515, 625, 250, 50}    // 3 - Sair
};


// =====================================================
// INICIALIZAR JOGO
// =====================================================

void InitGame(GameState *game)
{
    game->currentScreen = SCREEN_MENU;

    game->currentScene = SCENE_PRESENT;

    game->dialogueIndex = 0;

    game->shouldExit = false;

    game->choice = 0;

    game->senhaNoQuarto = false;

    game->iaEscreveuDiscurso = false;
    game->campanha = CAMPANHA_EM_ANDAMENTO;
    game->motivoFinalRuim = FINAL_RUIM_IA_LEU;

    fadeFase = FADE_NENHUM;
    pcAtivo = false;

    Tokens_Reiniciar();
    BarraTokens_Reiniciar();
    Senha_ComecarJogo();
}


// =====================================================
// TROCAR DE CENA
// =====================================================

#ifdef RE7_DEV_MODE
static void DevRegistrarEntradaNoAto(const GameState *game);
#endif

void ChangeScene(
    GameState *game,
    SceneID nextScene
)
{
    game->currentScene = nextScene;

    game->dialogueIndex = 0;

#ifdef RE7_DEV_MODE
    // modo desenvolvedor: guarda tokens e variaveis da entrada em cada ato
    // (para "Reiniciar ato")
    DevRegistrarEntradaNoAto(game);
#endif
}


// =====================================================
// COMECAR NOVO JOGO
// (usada no menu e na tela final, para nao repetir o bloco)
// =====================================================

static void ComecarNovoJogo(GameState *game)
{
    game->currentScreen = SCREEN_GAME;

    ChangeScene(game, SCENE_PRESENT);

    flashTimer = 0.0f;
    fadeFase = FADE_NENHUM;
    pcAtivo = false;

    game->choice = 0;
    game->senhaNoQuarto = false;
    game->iaEscreveuDiscurso = false;
    game->campanha = CAMPANHA_EM_ANDAMENTO;

    Tokens_Reiniciar();                             // barra volta a 3
    BarraTokens_Reiniciar();
    Senha_ComecarJogo();

#ifdef RE7_DEV_MODE
    DevRegistrarEntradaNoAto(game);                 // a barra ja voltou a 3
#endif

    SaveGame(game);                                 // zera o save
}


// =====================================================
// FIM DE CADA MINIJOGO E AVANCO DAS FALAS
// (usados pelo jogo e pelo modo desenvolvedor, para nao repetir a logica)
// =====================================================

static void ConcluirSenha(GameState *game)
{
    game->currentScreen = SCREEN_GAME;
    ChangeScene(game, SCENE_ATO1_IA_A);
    SaveGame(game);
}

static void ConcluirCorrida(GameState *game)
{
    // chegou ao fim do corredor
    game->currentScreen = SCREEN_GAME;
    ChangeScene(game, SCENE_ATO2_ENCONTRO);
    encontroTempo = 0.0f;
    IniciarFade(FADE_CLAREANDO, FADE_ENTRADA_ENCONTRO, SCENE_ATO2_ENCONTRO);
    SaveGame(game);
}

static void ConcluirQuarto(GameState *game)
{
    // Fim da exploracao: o PC esta bloqueado
    game->currentScreen = SCREEN_GAME;
    ChangeScene(game, SCENE_2007_PC);
    SaveGame(game);
}

static void ConcluirEntreLinhas(GameState *game)
{
    // as tres palavras recuperadas: a plateia aplaude e a Lia fala
    game->currentScreen = SCREEN_GAME;
    ChangeScene(game, Ato3_CenaDeConclusao(game->iaEscreveuDiscurso));
    SaveGame(game);
}

// Proxima fala (clique ou ESPACO). No fim da cena, FimDaCena decide o que vem.
static void AvancarFala(GameState *game)
{
    Scene scene = GetScene(game->currentScene);

    game->dialogueIndex++;

    if (game->dialogueIndex >= scene.dialogueCount)
    {
        FimDaCena(game, scene);

        if (game->currentScreen != SCREEN_GAME || game->choice != ESCOLHA_NENHUMA)
            return;
    }

    SaveGame(game);                                 // salva a cada fala
}


// =====================================================
// UPDATE
// =====================================================

typedef enum
{
    PAUSA_VOLTAR,
    PAUSA_AUDIO,
    PAUSA_SALVAR,
    PAUSA_SAIR
} AcaoPausa;

static const struct
{
    Rectangle area;
    const char *rotulo;
} opcoesPausa[] =
{
    [PAUSA_VOLTAR] = {{430, 246, 420, 52}, "Voltar ao menu"},
    [PAUSA_AUDIO]  = {{430, 318, 420, 52}, "Áudio"},
    [PAUSA_SALVAR] = {{430, 390, 420, 52}, "Salvar Progresso"},
    [PAUSA_SAIR]   = {{430, 462, 420, 52}, "Sair do Jogo"}
};

static const Rectangle fecharPausa = {838, 135, 36, 36};
static const Rectangle cancelarRetorno = {390, 387, 230, 52};
static const Rectangle confirmarRetorno = {660, 387, 230, 52};
static const char *mensagemPausa = NULL;

static bool PodePausar(GameScreen tela)
{
    return tela == SCREEN_GAME || tela == SCREEN_ROOM ||
           tela == SCREEN_SENHA || tela == SCREEN_CORRIDA ||
           tela == SCREEN_ENTRE_LINHAS;
}

static Rectangle AreaBotaoMenu(GameScreen tela)
{
    const float lado = 44.0f;      // icone de engrenagem
    const float margem = 14.0f;

    if (tela == SCREEN_ROOM)
        return (Rectangle){GetScreenWidth() - lado - margem, margem, lado, lado};

    // Na senha, o cabecalho ocupa a parte de cima da tela.
    float y = (tela == SCREEN_SENHA) ? 662.0f : margem;
    return (Rectangle){margem, y, lado, lado};
}

static void AbrirPausa(GameState *game)
{
    game->pauseReturnScreen = game->currentScreen;
    game->currentScreen = SCREEN_PAUSE;
    mensagemPausa = NULL;
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
}

static void AtualizarMenuPausa(GameState *game)
{
    if (game->currentScreen == SCREEN_CONFIRM_MENU)
    {
        if (IsButtonClicked(cancelarRetorno) || IsKeyPressed(KEY_ESCAPE))
        {
            AudioManager_Play(SOM_MOUSE);
            mensagemPausa = NULL;
            game->currentScreen = SCREEN_PAUSE;
        }
        else if (IsButtonClicked(confirmarRetorno))
        {
            AudioManager_Play(SOM_MOUSE);
            if (SaveGame(game))
                game->currentScreen = SCREEN_MENU;
            else
                mensagemPausa = "Não foi possível salvar. Tente novamente.";
        }

        return;
    }

    if (IsButtonClicked(fecharPausa) || IsKeyPressed(KEY_ESCAPE))
    {
        AudioManager_Play(SOM_MOUSE);
        game->currentScreen = game->pauseReturnScreen;
        return;
    }

    for (int i = 0; i < (int)(sizeof(opcoesPausa) / sizeof(opcoesPausa[0])); i++)
    {
        if (!IsButtonClicked(opcoesPausa[i].area)) continue;

        AudioManager_Play(SOM_MOUSE);
        switch (i)
        {
            case PAUSA_VOLTAR:
                mensagemPausa = NULL;
                game->currentScreen = SCREEN_CONFIRM_MENU;
                break;
            case PAUSA_AUDIO:
                game->currentScreen = SCREEN_AUDIO;
                break;
            case PAUSA_SALVAR:
                mensagemPausa = SaveGame(game)
                    ? "Progresso salvo."
                    : "Não foi possível salvar. Tente novamente.";
                break;
            case PAUSA_SAIR:
                game->shouldExit = true;
                break;
        }

        return;
    }
}

void UpdateGame(GameState *game)
{
    // o flash de viagem no tempo vai se apagando
    if (flashTimer > 0.0f) flashTimer -= GetFrameTime();

    // F2 liga e desliga o som em qualquer tela
    if (IsKeyPressed(KEY_F2))
    {
        AudioManager_ToggleMute();
        avisoSom = 1.5f;
    }

    if (avisoSom > 0.0f)
        avisoSom -= GetFrameTime();

    // Atualiza a musica de fundo
    AudioManager_Update();
    AtualizarMusica(game);

    // Animacao da barra de tokens
    BarraTokens_Atualizar();

    if (PodePausar(game->currentScreen) &&
        IsButtonClicked(AreaBotaoMenu(game->currentScreen)))
    {
        AudioManager_Play(SOM_MOUSE);
        AbrirPausa(game);
        return;
    }

    if (game->currentScreen == SCREEN_PAUSE ||
        game->currentScreen == SCREEN_CONFIRM_MENU)
    {
        AtualizarMenuPausa(game);
        return;
    }

    // Painel de configuracoes de audio
    if (game->currentScreen == SCREEN_AUDIO)
    {
        Vector2 mouse = GetMousePosition();

        if (IsButtonClicked(audioFechar))
        {
            game->currentScreen = SCREEN_PAUSE;
            return;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (CheckCollisionPointRec(mouse, sliderMusica))
            {
                float volume = (mouse.x - sliderMusica.x) /
                               sliderMusica.width;
                AudioManager_SetMusicVolume(volume);
            }

            if (CheckCollisionPointRec(mouse, sliderEfeitos))
            {
                float volume = (mouse.x - sliderEfeitos.x) /
                               sliderEfeitos.width;
                AudioManager_SetEffectsVolume(volume);
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(mouse, audioMudo))
        {
            AudioManager_ToggleMusicMute();
            AudioManager_Play(SOM_MOUSE);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(mouse, audioTestar))
        {
            AudioManager_Play(SOM_MOUSE);
        }

        return;
    }

    // -------------------------------------------------
    // MENU
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_MENU)
    {
        if (IsButtonClicked(menuRects[0]))          // Novo Jogo
        {
            AudioManager_Play(SOM_MOUSE);
            ComecarNovoJogo(game);
        }

        if (IsButtonClicked(menuRects[1]))          // Continuar
        {
            AudioManager_Play(SOM_MOUSE);
            
            // carrega o save; se nao houver, comeca do inicio
            if (!LoadGame(game))
            {
                ChangeScene(game, SCENE_PRESENT);
            }

            flashTimer = 0.0f;

            game->choice = 0;

            game->currentScreen = SCREEN_GAME;
        }

        if (IsButtonClicked(menuRects[2]))          // Creditos
        {
            AudioManager_Play(SOM_MOUSE);
            
            game->currentScreen = SCREEN_CREDITS;
        }

        if (IsButtonClicked(menuRects[3]))          // Sair
        {
            AudioManager_Play(SOM_MOUSE);
            
            game->shouldExit = true;
        }

        return;
    }


    // -------------------------------------------------
    // CREDITOS
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CREDITS)
    {
        if (
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            ||
            IsKeyPressed(KEY_ENTER)
            ||
            IsKeyPressed(KEY_BACKSPACE)
            ||
            IsKeyPressed(KEY_ESCAPE)
        )
        {
            game->currentScreen = SCREEN_MENU;
        }

        return;
    }


    // -------------------------------------------------
    // TELA FINAL: jogar de novo, menu ou sair
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ENDING)
    {
        if (avisoInvalido > 0.0f) avisoInvalido -= GetFrameTime();

        int tecla;                  // tecla apertada neste quadro

        // le TODAS as teclas apertadas neste quadro
        // e valida cada uma (so ENTER, M e ESC sao aceitas)
        while ((tecla = GetKeyPressed()) != 0)
        {
            if (tecla == KEY_ENTER)
            {
                ComecarNovoJogo(game);

                avisoInvalido = 0.0f;
            }
            else if (tecla == KEY_M)
            {
                game->currentScreen = SCREEN_MENU;

                avisoInvalido = 0.0f;
            }
            else if (tecla == KEY_ESCAPE)
            {
                game->shouldExit = true;
            }
            else
            {
                avisoInvalido = 1.5f;               // opcao invalida
            }
        }

        return;
    }


    // -------------------------------------------------
    // SENHA (Ato 1)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_SENHA)
    {
        Senha_Atualizar();

        if (Senha_Concluida())
            ConcluirSenha(game);

        return;
    }


    // -------------------------------------------------
    // CORRIDA DE OBSTACULOS (Ato 2)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CORRIDA)
    {
        MinigameResult resultado = UpdateAto2Corrida();

        if (resultado == MINIGAME_VENCEU)      // chegou ao fim do corredor
            ConcluirCorrida(game);

        return;
    }


    // -------------------------------------------------
    // DESAFIO 3: ENTRE AS LINHAS (Ato 3)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ENTRE_LINHAS)
    {
        // ESC: abre o menu de pausa
        if (IsKeyPressed(KEY_ESCAPE))
        {
            AudioManager_Play(SOM_MOUSE);
            AbrirPausa(game);
            return;
        }

        if (UpdateAto3EntreLinhas() == MINIGAME_VENCEU)
            ConcluirEntreLinhas(game);

        return;
    }


    // -------------------------------------------------
    // EXPLORACAO DO QUARTO
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ROOM)
    {
        RoomResult resultadoQuarto = UpdateRoom();

        if (resultadoQuarto == ROOM_CONCLUIDO)
            ConcluirQuarto(game);

        return;
    }


    // -------------------------------------------------
    // JOGO (HISTORIA)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_GAME)
    {
        // ESC: abre o menu de pausa (o mesmo da engrenagem)
        if (IsKeyPressed(KEY_ESCAPE))
        {
            AudioManager_Play(SOM_MOUSE);
            AbrirPausa(game);
            return;
        }

        // Sequencia do computador (Doors XP): nada responde; ESPACO/ENTER (com a pagina aberta) segue para o Ato 2
        if (pcAtivo)
        {
            if (pcTempo < PC_ESPERA) pcTempo += GetFrameTime();
            if (pcTempo > PC_ESPERA) pcTempo = PC_ESPERA;

            // ESPACO/ENTER: depois que a pagina abriu, escurece e entra no Ato 2
            if (pcSaida < 0.0f && pcTempo >= PC_PERFIL_INICIO + PC_PERFIL_FADE &&
                (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)))
                pcSaida = 0.0f;

            if (pcSaida >= 0.0f) pcSaida += GetFrameTime();

            if (pcSaida >= PC_SAIDA)
            {
                pcAtivo = false;
                ChangeScene(game, SCENE_ATO2_QUARTO);
                SaveGame(game);
                IniciarFade(FADE_CLAREANDO, FADE_ENTRADA_LANHOUSE, SCENE_ATO2_QUARTO);
            }
            return;
        }

        // Tempo do encontro (o amigo vira para olhar a Lia)
        if (game->currentScene == SCENE_ATO2_ENCONTRO)
            encontroTempo += GetFrameTime();
        else
            encontroTempo = 0.0f;

        // Fade para preto: enquanto escurece, nada responde; no fim troca de cena
        if (fadeFase != FADE_NENHUM)
        {
            fadeTempo += GetFrameTime();

            if (fadeFase == FADE_ESCURECENDO && fadeTempo >= fadeDuracao && fadeDestino == SCENE_ATO2_QUARTO)
            {
                // fim do Ato 1: antes do Ato 2 roda a sequencia do computador (Doors XP)
                fadeFase = FADE_NENHUM;
                pcAtivo = true;
                pcTempo = 0.0f;
                pcSaida = -1.0f;
                return;
            }

            if (fadeFase == FADE_ESCURECENDO && fadeTempo >= fadeDuracao)
            {
                ChangeScene(game, fadeDestino);
                SaveGame(game);

                if (SegurarPretoAntes(fadeDestino))
                    IniciarFade(FADE_SEGURANDO, FADE_TEXTO_SEGUNDOS, fadeDestino);
                else
                    IniciarFade(FADE_CLAREANDO, FADE_ENTRADA_LANHOUSE, fadeDestino);
            }
            else if (fadeFase == FADE_SEGURANDO && fadeTempo >= fadeDuracao)
            {
                IniciarFade(FADE_CLAREANDO, FADE_ENTRADA_LANHOUSE, fadeDestino);
            }
            else if (fadeFase == FADE_CLAREANDO && fadeTempo >= fadeDuracao)
            {
                fadeFase = FADE_NENHUM;
            }

            if (fadeFase == FADE_ESCURECENDO || fadeFase == FADE_SEGURANDO) return;
        }

        // Enquanto ha uma escolha na tela, so as opcoes respondem
        if (game->choice != ESCOLHA_NENHUMA)
        {
            AtualizarEscolha(game);

            return;
        }

        if (IsKeyPressed(KEY_SPACE) ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            AvancarFala(game);
        }
    }
}


static void DesenharBotaoPausa(Rectangle area, const char *texto, int tamanho)
{
    bool sobre = CheckCollisionPointRec(GetMousePosition(), area);
    Color fundo = sobre ? (Color){113, 46, 110, 255} : (Color){31, 39, 77, 255};
    Color borda = sobre ? (Color){255, 105, 200, 255} : Fade(SKYBLUE, 0.5f);

    DrawRectangleRounded(area, 0.28f, 8, fundo);
    DrawRectangleRoundedLines(area, 0.28f, 8, borda);

    int x = (int)(area.x + (area.width - UiMeasureTextEmphasis(texto, tamanho)) / 2.0f);
    int y = (int)(area.y + (area.height - tamanho) / 2.0f);
    UiDrawTextEmphasis(texto, x, y, tamanho, WHITE);
}

// Engrenagem desenhada em codigo: discreta, ganha destaque ao passar o mouse
static void DesenharIconeEngrenagem(Rectangle area, bool sobre)
{
    Vector2 c = { area.x + area.width / 2.0f, area.y + area.height / 2.0f };
    Color cor = sobre ? (Color){255, 105, 200, 255} : Fade(WHITE, 0.6f);

    DrawCircleV(c, area.width / 2.0f, Fade(BLACK, sobre ? 0.55f : 0.35f));

    DrawRing(c, 6.0f, 12.0f, 0.0f, 360.0f, 40, cor);              // corpo com furo no meio

    for (int i = 0; i < 8; i++)                                   // 8 dentes
    {
        float ang = i * 45.0f * DEG2RAD;
        Vector2 ponta = { c.x + cosf(ang) * 12.0f, c.y + sinf(ang) * 12.0f };
        DrawRectanglePro((Rectangle){ ponta.x, ponta.y, 6.0f, 6.0f },
                         (Vector2){ 3.0f, 3.0f }, i * 45.0f, cor);
    }
}

void DrawPauseControls(GameState *game)
{
    if (!PodePausar(game->currentScreen)) return;

    Rectangle area = AreaBotaoMenu(game->currentScreen);
    bool sobre = CheckCollisionPointRec(GetMousePosition(), area);

    DesenharIconeEngrenagem(area, sobre);

    // dica ao passar o mouse
    if (sobre)
    {
        const char *dica = "Menu (ESC)";
        int larg = UiMeasureText(dica, 18);
        float x = (game->currentScreen == SCREEN_ROOM) ? area.x - larg - 30.0f : area.x + area.width + 8.0f;
        Rectangle caixa = { x, area.y + 7.0f, larg + 20.0f, 30.0f };

        DrawRectangleRounded(caixa, 0.4f, 8, Fade(BLACK, 0.7f));
        UiDrawText(dica, (int)caixa.x + 10, (int)caixa.y + 5, 18, WHITE);
    }
}

static void DesenharMenuPausa(void)
{
    DesenharFundo();

    Rectangle painel = {382, 112, 516, 510};
    DrawRectangleRounded(painel, 0.06f, 12, (Color){18, 20, 45, 245});
    DrawRectangleRoundedLines(painel, 0.06f, 12, Fade(SKYBLUE, 0.55f));

    DrawCenteredText("Jogo pausado", 153, 42, WHITE);
    DrawLine(430, 223, 850, 223, Fade(SKYBLUE, 0.35f));
    DesenharBotaoPausa(fecharPausa, "X", 18);

    for (int i = 0; i < (int)(sizeof(opcoesPausa) / sizeof(opcoesPausa[0])); i++)
        DesenharBotaoPausa(opcoesPausa[i].area, opcoesPausa[i].rotulo, 24);

    if (mensagemPausa)
        DrawCenteredText(mensagemPausa, 537, 19, SKYBLUE);

    DrawCenteredText("Clique no X ou aperte ESC para retomar", 583, 18, Fade(WHITE, 0.7f));
}

static void DesenharConfirmacaoMenu(void)
{
    DesenharFundo();

    Rectangle painel = {314, 221, 652, 275};
    DrawRectangleRounded(painel, 0.10f, 12, (Color){18, 20, 45, 245});
    DrawRectangleRoundedLines(painel, 0.10f, 12, Fade(SKYBLUE, 0.55f));

    DrawCenteredText("Voltar ao menu principal?", 260, 34, WHITE);
    DrawCenteredText("Seu progresso será salvo antes de voltar.", 326, 22, SKYBLUE);

    DesenharBotaoPausa(cancelarRetorno, "Cancelar", 22);
    DesenharBotaoPausa(confirmarRetorno, "Confirmar", 22);

    if (mensagemPausa)
        DrawCenteredText(mensagemPausa, 454, 18, PINK);
}

// =====================================================
// ATO 3: AUDITORIO
// =====================================================

static bool CenaDoAto3(SceneID cena)
{
    // as cenas do Ato 3 ficam no fim da lista (depois do epilogo)
    return cena >= SCENE_ATO3_ABERTURA && cena < SCENE_COUNT;
}

static void DesenharCenaAto3(GameState *game, Assets *assets, const Scene *scene, Dialogue dialogue)
{
    // tempo desde que a cena comecou (o PC pisca no comeco da reserva,
    // a camera se aproxima da Lia na conclusao)
    static SceneID cenaAnterior = (SceneID)-1;
    static float tempoNaCena = 0.0f;

    if (game->currentScene != cenaAnterior)
    {
        cenaAnterior = game->currentScene;
        tempoNaCena = 0.0f;
    }

    if (fadeFase != FADE_SEGURANDO) tempoNaCena += GetFrameTime();

    SceneID c = game->currentScene;
    bool conclusao = (c == SCENE_ATO3_CONCLUSAO || c == SCENE_ATO3_CONCLUSAO_IA_ESCREVEU ||
                      c == SCENE_ATO3_ETAPA_CONCLUIDA);
    bool iaLeu = (c == SCENE_ATO3_IA_LE || c == SCENE_ATO3_IA_LE_IA_ESCREVEU);
    bool antesDaEscolha = (c == SCENE_ATO3_ABERTURA || c == SCENE_ATO3_RESERVA || c == SCENE_ATO3_OFERTA);

    float foco = tempoNaCena / 1.2f;
    if (foco > 1.0f) foco = 1.0f;

    // Expressao da Lia no pulpito: a da ultima fala dela. Sem expressao
    // marcada (ou "preocupada", que so existe na Lia de 2026), ela fica
    // pensativa enquanto esta nervosa.
    Pose poseLia = PoseDe(scene, game->dialogueIndex, "Lia");
    if (poseLia == POSE_PREOCUPADA || (poseLia == POSE_PADRAO && antesDaEscolha))
        poseLia = POSE_PENSATIVA;

    Ato3Cena cena = {
        .pcOnline = !Tokens_IAOffline(),
        .pcPiscando = (c == SCENE_ATO3_RESERVA && tempoNaCena < 1.2f),
        // cenario 1: a plateia aplaude no fim, mas a Lia nao sorri
        .aplausos = conclusao || (iaLeu && game->dialogueIndex >= scene->dialogueCount - 1),
        .foco = conclusao ? foco * (2.0f - foco) : 0.0f,
        .nervosismo = antesDaEscolha ? 1.0f : ((c == SCENE_ATO3_AJUDA || c == SCENE_ATO3_SOZINHA) ? 0.6f : 0.0f),
        .lia = TexturaLia(assets, 2007, poseLia)
    };

    Ato3_DesenharCenario(&cena, assets);

    // Papel: aparece depois da fala da Lia; na conclusao a camera fica na Lia.
    bool mostrarPapel = !conclusao && !(c == SCENE_ATO3_ABERTURA && game->dialogueIndex < 1);

    if (mostrarPapel)
    {
        // cenario 1: o papel fica nitido quando a IA le as palavras
        float n = (iaLeu && game->dialogueIndex >= 1) ? 1.0f : 0.0f;
        float nitidez[3] = { n, n, n };

        // a esquerda do palco: a Lia e o PC do pulpito continuam visiveis
        Rectangle papel = {70, 175, 400, 232};
        Ato3_DesenharPapel(papel, 19, nitidez, -1, (Vector2){0, 0}, assets->papelDiscurso);

        if (n < 1.0f)
            UiDrawText("As palavras parecem embaralhadas...", (int)papel.x + 27,
                       (int)(papel.y + papel.height) - 30, 17, (Color){120, 88, 77, 255});
    }

    DrawDialogueBox(dialogue.speaker, dialogue.text);
    BarraTokens_Desenhar(990, 20);

    if (game->choice == ESCOLHA_NENHUMA)
        UiDrawText("CLIQUE ou ESPAÇO para continuar", 760, 690, 18, Fade(WHITE, 0.8f));
    else
        DesenharEscolha(game);

    DesenharFlash();
    DesenharFade();
    DesenharAvisoSom();
}


// =====================================================
// TELA FINAL (final bom ou ruim)
// =====================================================

static void DesenharOpcoesDaTelaFinal(int y)
{
    DrawCenteredText("ENTER - jogar novamente", y, 28, WHITE);
    DrawCenteredText("M - voltar ao menu", y + 45, 28, WHITE);
    DrawCenteredText("ESC - sair do jogo", y + 90, 28, WHITE);

    if (avisoInvalido > 0.0f)
    {
        DrawCenteredText(
            "Opcao invalida! Use ENTER, M ou ESC.",
            y + 160, 24,
            Fade((Color){255, 105, 200, 255}, fminf(1.0f, avisoInvalido))
        );
    }
}

// Final ruim (roteiro do Ato 3): MSN offline e o cartao
static void DesenharFinalRuim(const GameState *game)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){10, 10, 18, 255});

    // janela do MSN, offline
    Rectangle janela = {340, 70, 600, 180};
    DrawRectangleRounded(janela, 0.06f, 8, (Color){210, 214, 222, 255});
    DrawRectangleRounded((Rectangle){janela.x, janela.y, janela.width, 40}, 0.4f, 8, (Color){120, 128, 140, 255});
    DrawCircle((int)janela.x + 24, (int)janela.y + 20, 8, (Color){200, 60, 90, 255});
    UiDrawText("IA - Offline", (int)janela.x + 42, (int)janela.y + 9, 22, WHITE);

    UiDrawTextEmphasis("MSN OFFLINE", (int)janela.x + 30, (int)janela.y + 64, 30, (Color){29, 38, 96, 255});
    UiDrawText(ATO3_MSN_OFFLINE, (int)janela.x + 30, (int)janela.y + 112, 22, (Color){70, 70, 90, 255});

    // cartao
    const char *cartao = (game->motivoFinalRuim == FINAL_RUIM_SEM_TOKENS)
        ? ATO3_CARTAO_SEM_TOKEN : ATO3_CARTAO_IA_LEU;

    int fs = 34;
    int larg = UiMeasureTextEmphasis(cartao, fs);
    UiDrawTextEmphasis(cartao, (GetScreenWidth() - larg) / 2, 300, fs, WHITE);

    DrawCenteredText("FIM (FINAL RUIM)", 365, 26, (Color){255, 105, 200, 255});

    DesenharOpcoesDaTelaFinal(450);
}

static void DesenharFinalBom(void)
{
    DesenharFundo();

    DrawCenteredText("FIM", 150, 90, WHITE);
    DrawCenteredText("Lia aprendeu a confiar na propria voz.", 280, 30, SKYBLUE);
    DrawCenteredText("Obrigada por jogar Re:7!", 330, 28, WHITE);

    DesenharOpcoesDaTelaFinal(440);
}


// =====================================================
// DESENHAR JOGO
// =====================================================

void DrawGame(
    GameState *game,
    Assets *assets
)
{

    if (game->currentScreen == SCREEN_PAUSE)
    {
        DesenharMenuPausa();
        return;
    }

    if (game->currentScreen == SCREEN_CONFIRM_MENU)
    {
        DesenharConfirmacaoMenu();
        return;
    }

    // Painel de configuracoes de audio
    if (game->currentScreen == SCREEN_AUDIO)
    {
        DrawRectangle(
            0, 0,
            GetScreenWidth(), GetScreenHeight(),
            (Color){15, 15, 35, 245}
        );

        UiDrawText("CONFIGURACOES DE AUDIO",
                 350, 100, 32, WHITE);

        DesenharSlider(
            sliderMusica,
            AudioManager_GetMusicVolume(),
            "Volume da musica"
        );

        DesenharSlider(
            sliderEfeitos,
            AudioManager_GetEffectsVolume(),
            "Volume dos efeitos"
        );

        DrawRectangleRec(audioTestar, DARKBLUE);
        UiDrawText("TESTAR EFEITO", 500, 447, 20, WHITE);

        DrawRectangleRec(audioMudo, DARKPURPLE);
        UiDrawText(
            AudioManager_IsMusicMuted()
                ? "REATIVAR MUSICA"
                : "SILENCIAR MUSICA",
            485, 517, 18, WHITE
        );

        DrawRectangleRec(audioFechar, MAROON);
        UiDrawText("VOLTAR", 1055, 93, 18, WHITE);

        return;
    }

    // -------------------------------------------------
    // MENU
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_MENU)
    {
        DesenharFundo();

        // X, Y, largura, altura
        // (aumentar o Y desce o logo, diminuir sobe)
        Rectangle logoRect = {280, 100, 719, 319};

        DrawTexturePro(
            assets->logo,
            (Rectangle){
                0,
                0,
                assets->logo.width,
                assets->logo.height
            },
            logoRect,
            (Vector2){0, 0},
            0.0f,
            WHITE
        );

        for (int i = 0; i < MENU_BUTTONS; i++)
        {
            DrawImageButton(
                assets->menuButtons[i][0],
                assets->menuButtons[i][1],
                assets->menuButtons[i][2],
                menuRects[i]
            );
        }

        return;
    }


    // -------------------------------------------------
    // CREDITOS
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CREDITS)
    {
        DesenharFundo();

        DrawCenteredText("CREDITOS", 50, 48, WHITE);
        DrawCenteredText("Re:7 - Projeto Integrador", 115, 26, SKYBLUE);

        // nomes da equipe (um por linha, centralizados)
        static const char *nomesEquipe[] =
        {
            "Aline Santos",
            "Ana Rita Correia",
            "Gabriel Farias",
            "Juan Nunes",
            "Juliana Freire",
            "Leandro Luís",
            "Luiz Mario",
            "Vinícius Lino",
            "Vitória Santos"
        };

        int totalNomes = (int)(sizeof(nomesEquipe) / sizeof(nomesEquipe[0]));

        for (int i = 0; i < totalNomes; i++)
        {
            DrawCenteredText(nomesEquipe[i], 190 + i * 41, 30, WHITE);
        }

        DrawCenteredText("Clique, ENTER ou ESC para voltar", 640, 20, Fade(WHITE, 0.7f));

        return;
    }


    // -------------------------------------------------
    // TELA FINAL
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ENDING)
    {
        if (game->campanha == CAMPANHA_FINAL_RUIM)
            DesenharFinalRuim(game);
        else
            DesenharFinalBom();

        return;
    }


    // -------------------------------------------------
    // DESAFIO 3: ENTRE AS LINHAS (Ato 3)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ENTRE_LINHAS)
    {
        DrawAto3EntreLinhas(assets);
        BarraTokens_Desenhar(990, 20);
        DesenharAvisoSom();
        return;
    }


    // -------------------------------------------------
    // SENHA (Ato 1)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_SENHA)
    {
        Senha_Desenhar();

        BarraTokens_Desenhar(990, 20);

        return;
    }


    // -------------------------------------------------
    // CORRIDA DE OBSTACULOS (Ato 2)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CORRIDA)
    {
        DrawAto2Corrida();

        return;
    }


    // -------------------------------------------------
    // EXPLORACAO DO QUARTO
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ROOM)
    {
        DrawRoom(assets);
        return;
    }


    // -------------------------------------------------
    // CENAS
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_GAME)
    {
        Scene scene = GetScene(game->currentScene);

        Dialogue dialogue = scene.dialogues[game->dialogueIndex];

        if (CenaDoAto3(game->currentScene))
        {
            DesenharCenaAto3(game, assets, &scene, dialogue);
            return;
        }

        VisualCena visual = VisualDaCena(game->currentScene);

        // Fundo

        switch (visual.fundo)
        {
            case FUNDO_2026:          DesenharImagemTela(assets->bgPresent);     break;
            case FUNDO_2007:          DesenharImagemTela(assets->bg2007);        break;
            case FUNDO_CORREDOR:      DesenharImagemTela(assets->bgCorredor);    break;
            case FUNDO_LANHOUSE:      DesenharImagemTela(assets->bgLanhouse);    break;
            case FUNDO_CG_TELA:       DesenharImagemTela(assets->cgTela2026);    break;
            case FUNDO_CG_ESCREVEM:   DesenharImagemZoom(assets->cgEscrevem, game->currentScene, 0.65f, 0.40f);    break;
            case FUNDO_CG_PROFESSORA: DesenharImagemZoom(assets->cgProfessora, game->currentScene, 0.45f, 0.40f);  break;
            case FUNDO_CG_ENSAIO:     DesenharImagemZoom(assets->cgEnsaio, game->currentScene, 0.70f, 0.40f);      break;
            case FUNDO_CG_CASA_NOITE:    DesenharImagemZoom(assets->cgCasaNoite, game->currentScene, 0.70f, 0.40f);    break;
            case FUNDO_CG_FRENTE_ESCOLA: DesenharImagemZoom(assets->cgFrenteEscola, game->currentScene, 0.33f, 0.55f); break;
        }

        // Personagens: o amigo a esquerda, a Lia a direita
        // (altura, posicao Y do topo, borda direita)

        // O Ato 2 usa o layout do Figma; o Ato 1 continua como estava
        bool noAto2 = game->currentScene >= SCENE_ATO2_QUARTO && game->currentScene < SCENE_EPILOGUE;

        // Na cena 3.2 (lan house sem o Joao) ele so entra na segunda fala
        bool mostrarAmigo = visual.amigo;

        if (game->currentScene == SCENE_ATO2_LANHOUSE_SEM && game->dialogueIndex < 1)
            mostrarAmigo = false;

        if (mostrarAmigo)
        {
            Pose poseAmigo = PoseDe(&scene, game->dialogueIndex, "João");

            Texture2D texAmigo = assets->amigo[poseAmigo == POSE_FELIZ ? 1 : 0];

            // Encontro: primeiro ele olha para o outro lado, depois vira e olha para a Lia
            if (game->currentScene == SCENE_ATO2_ENCONTRO)
                texAmigo = assets->amigoSurpreso[encontroTempo < ENCONTRO_VIRA_SEGUNDOS ? 0 : 1];

            if (noAto2)
                DrawPersonagemEsq(texAmigo, ATO2_JOAO_ALTURA, ATO2_JOAO_Y, ATO2_JOAO_X, false);
            else
                DrawPersonagem(texAmigo, 900.0f, -101.0f, 610.0f);
        }

        if (visual.lia != 0)
        {
            Pose poseLia = PoseDe(&scene, game->dialogueIndex, "Lia");

            // Escolha 1 (corrida): a Lia aparece determinada
            if (game->currentScene == SCENE_ATO2_DETERMINADA)
                poseLia = POSE_DETERMINADA;

            // Reacao da Lia numa fala da IA (se a fala da IA tiver uma expressao, ela aparece)
            else if (strcmp(dialogue.speaker, "IA") == 0 && dialogue.pose != POSE_PADRAO)
                poseLia = dialogue.pose;

            Texture2D texLia = TexturaLia(assets, visual.lia, poseLia);

            // No Ato 2 a Lia fica a direita e deve olhar para a esquerda (para o Joao).
            // Estas 4 imagens olham para a direita, entao so no Ato 2 viram na horizontal.
            bool poseOlhaDireita = (poseLia == POSE_PADRAO || poseLia == POSE_CONFUSA ||
                                    poseLia == POSE_BRAVA  || poseLia == POSE_SURPRESA);

            // Lia de 2007: mesmo tamanho e posicao em todos os atos.
            // So no Ato 2 ela e espelhada (para olhar para o Joao).
            if (visual.lia == 2007)
                DrawPersonagemEsq(texLia, ATO2_LIA_ALTURA, ATO2_LIA_Y, ATO2_LIA_X, noAto2 && poseOlhaDireita);
            else
                DrawPersonagem(texLia, 820.0f, -7.0f, 1250.0f);
        }

        // ---------------------------------------------
        // DIÁLOGO
        // ---------------------------------------------

        DrawDialogueBox(
            dialogue.speaker,
            dialogue.text
        );

        // imagens de cena (sem fala): dica para continuar
        if (dialogue.text[0] == '\0' && dialogue.speaker[0] == '\0' && game->choice == ESCOLHA_NENHUMA)
        {
           UiDrawText("CLIQUE ou ESPACO  >>", 900, 672, 22, Fade(WHITE, 0.85f));
        }

        // ---------------------------------------------
        // AVATAR DA IA (so quando a IA esta falando)
        // ---------------------------------------------

        if (strcmp(dialogue.speaker, "IA") == 0 && game->choice == ESCOLHA_NENHUMA)
        {
            DesenharAvatarIA(assets);
        }

        // ---------------------------------------------
        // BARRA DE TOKENS (depois da senha, ate o epilogo)
        // ---------------------------------------------

        if (game->currentScene >= SCENE_ATO1_IA_A && game->currentScene < SCENE_EPILOGUE)
        {
            BarraTokens_Desenhar(990, 20);
        }

        // ---------------------------------------------
        // ESCOLHA
        // ---------------------------------------------

        if (game->choice != ESCOLHA_NENHUMA)
        {
            DesenharEscolha(game);
        }

        // ---------------------------------------------
        // FLASH DA VIAGEM NO TEMPO
        // ---------------------------------------------

        DesenharFlash();

        // Fade para preto (por cima de tudo, menos do aviso de som)
        DesenharFade();

        // Sequencia do computador por cima de tudo
        if (pcAtivo) PcDesenhar(assets);

        // Exibe o aviso de som por cima da tela atual
        DesenharAvisoSom();
    }


    // Para desenvolvimento:

    // DrawDebugGrid();

    // DrawMousePosition();
}


// =====================================================
// MODO DESENVOLVEDOR (so no build com -DRE7_DEV_MODE)
// Atalhos para os testes. Tudo passa pelas mesmas funcoes do jogo
// (ChangeScene, AvancarFala, Concluir..., EncerrarCampanha, ComecarNovoJogo).
// =====================================================

#ifdef RE7_DEV_MODE

// Tokens e variaveis no momento em que cada ato comecou ("Reiniciar ato")
typedef struct
{
    bool valido;
    int tokens;
    bool reservaUsada;
    bool iaEscreveu;
} EntradaAto;

static EntradaAto entradaAto[ATO_EPILOGO + 1];

static AtoId AtoDaCena(SceneID cena)
{
    if (cena >= SCENE_ATO3_ABERTURA) return ATO_3;
    if (cena == SCENE_EPILOGUE)      return ATO_EPILOGO;
    if (cena >= SCENE_ATO2_QUARTO)   return ATO_2;
    return ATO_1;
}

static SceneID InicioDoAto(AtoId ato)
{
    switch (ato)
    {
        case ATO_2:       return SCENE_ATO2_QUARTO;
        case ATO_3:       return SCENE_ATO3_ABERTURA;
        case ATO_EPILOGO: return SCENE_EPILOGUE;
        default:          return SCENE_PRESENT;
    }
}

static void DevRegistrarEntradaNoAto(const GameState *game)
{
    for (int a = ATO_1; a <= ATO_EPILOGO; a++)
    {
        if (game->currentScene != InicioDoAto((AtoId)a)) continue;

        entradaAto[a].valido = true;
        entradaAto[a].tokens = Tokens_Valor();
        entradaAto[a].reservaUsada = Tokens_ReservaUsada();
        entradaAto[a].iaEscreveu = game->iaEscreveuDiscurso;
    }
}

AtoId Game_AtoAtual(const GameState *game)
{
    GameScreen tela = game->currentScreen;

    if (tela == SCREEN_PAUSE || tela == SCREEN_CONFIRM_MENU || tela == SCREEN_AUDIO)
        tela = game->pauseReturnScreen;

    switch (tela)
    {
        case SCREEN_ROOM:
        case SCREEN_SENHA:        return ATO_1;
        case SCREEN_CORRIDA:      return ATO_2;
        case SCREEN_ENTRE_LINHAS: return ATO_3;
        default:                  return AtoDaCena(game->currentScene);
    }
}

// Cancela fade, sequencia do PC, flash e escolha abertos
static void DevLimparTransicoes(GameState *game)
{
    fadeFase = FADE_NENHUM;
    pcAtivo = false;
    flashTimer = 0.0f;
    escolhaErro = 0.0f;
    avisoInvalido = 0.0f;
    encontroTempo = 0.0f;
    game->choice = ESCOLHA_NENHUMA;
}

void Game_IrParaAto(GameState *game, AtoId ato)
{
    DevLimparTransicoes(game);

    game->campanha = CAMPANHA_EM_ANDAMENTO;

    if (ato == ATO_1)
    {
        game->senhaNoQuarto = false;
        Senha_ComecarJogo();
    }

    game->currentScreen = SCREEN_GAME;
    ChangeScene(game, InicioDoAto(ato));        // guarda a entrada no ato

    if (ato == ATO_EPILOGO) flashTimer = FLASH_DURACAO;     // volta ao presente

    SaveGame(game);                             // save_dev.dat
}

void Game_ReiniciarAto(GameState *game)
{
    AtoId ato = Game_AtoAtual(game);

    if (entradaAto[ato].valido)
    {
        Tokens_Restaurar(entradaAto[ato].tokens, entradaAto[ato].reservaUsada);
        game->iaEscreveuDiscurso = entradaAto[ato].iaEscreveu;
    }

    Game_IrParaAto(game, ato);
}

void Game_ReiniciarCampanha(GameState *game)
{
    DevLimparTransicoes(game);
    ComecarNovoJogo(game);
}

// Termina na hora o fade ou a sequencia do PC. True se havia algum.
static bool DevPularTransicao(void)
{
    if (pcAtivo)
    {
        pcTempo = PC_ESPERA;
        if (pcSaida < PC_SAIDA) pcSaida = PC_SAIDA;
        return true;
    }

    if (fadeFase != FADE_NENHUM)
    {
        fadeTempo = fadeDuracao;
        return true;
    }

    return false;
}

bool Game_PularFala(GameState *game)
{
    switch (game->currentScreen)
    {
        case SCREEN_GAME:
            if (DevPularTransicao()) return true;
            if (game->choice != ESCOLHA_NENHUMA) return false;
            AvancarFala(game);
            return true;

        case SCREEN_ENTRE_LINHAS:
            return Ato3EntreLinhas_DevPular();

        case SCREEN_ROOM:
        case SCREEN_SENHA:
        case SCREEN_CORRIDA:
            return Game_AvancarEtapa(game);     // minijogos sem falas: conclui

        default:
            return false;
    }
}

bool Game_AvancarEtapa(GameState *game)
{
    switch (game->currentScreen)
    {
        case SCREEN_GAME:
        {
            if (DevPularTransicao()) return true;
            if (game->choice != ESCOLHA_NENHUMA) return false;

            Scene scene = GetScene(game->currentScene);
            game->dialogueIndex = scene.dialogueCount - 1;
            AvancarFala(game);                  // FimDaCena decide o que vem
            return true;
        }

        case SCREEN_ROOM:         ConcluirQuarto(game);   return true;
        case SCREEN_SENHA:        ConcluirSenha(game);    return true;
        case SCREEN_CORRIDA:      ConcluirCorrida(game);  return true;

        case SCREEN_ENTRE_LINHAS:
            Ato3EntreLinhas_DevResolverEtapa();     // a palavra certa
            return true;

        default:
            return false;
    }
}

void Game_ForcarFinal(GameState *game, EstadoCampanha final)
{
    DevLimparTransicoes(game);

    if (final == CAMPANHA_FINAL_BOM)
        EncerrarCampanha(game, CAMPANHA_FINAL_BOM, FINAL_RUIM_IA_LEU);
    else if (final == CAMPANHA_FINAL_RUIM)
        EncerrarCampanha(game, CAMPANHA_FINAL_RUIM, FINAL_RUIM_IA_LEU);
    else
        game->campanha = CAMPANHA_EM_ANDAMENTO;
}

const char *Game_NomeDaCena(SceneID cena)
{
    switch (cena)
    {
        case SCENE_PRESENT:                     return "Ato 1: presente (2026)";
        case SCENE_2007:                        return "Ato 1: quarto 2007";
        case SCENE_2007_PC:                     return "Ato 1: PC bloqueado";
        case SCENE_ATO1_IA_A:                   return "Ato 1: IA explica";
        case SCENE_ATO1_IA_B:                   return "Ato 1: ano certo";
        case SCENE_ATO2_QUARTO:                 return "Ato 2: quarto";
        case SCENE_ATO2_CORREDOR:               return "Ato 2: corredor";
        case SCENE_ATO2_DETERMINADA:            return "Ato 2: determinada";
        case SCENE_ATO2_LANHOUSE:               return "Ato 2: lan house";
        case SCENE_ATO2_IA_ESCREVE:             return "Ato 2: IA escreve";
        case SCENE_ATO2_UMA_IDEIA:              return "Ato 2: uma ideia";
        case SCENE_ATO2_COM_JOAO:               return "Ato 2: com o João";
        case SCENE_ATO2_C4:                     return "Ato 2: cena 4";
        case SCENE_ATO2_CG_ESCREVEM:            return "Ato 2: CG escrevem";
        case SCENE_ATO2_CG_PROFESSORA:          return "Ato 2: CG professora";
        case SCENE_ATO2_C4_ZERO:                return "Ato 2: cena 4 (barra 0)";
        case SCENE_ATO2_CG_ENSAIO:              return "Ato 2: CG ensaio";
        case SCENE_ATO2_ENCONTRO:               return "Ato 2: encontro";
        case SCENE_ATO2_RES21:                  return "Ato 2: resultado 2.1";
        case SCENE_ATO2_RES22:                  return "Ato 2: resultado 2.2";
        case SCENE_ATO2_LANHOUSE_SEM:           return "Ato 2: lan house sem João";
        case SCENE_ATO2_CG_PROFESSORA_SEM_IA:   return "Ato 2: CG professora sem IA";
        case SCENE_ATO2_CG_CASA_NOITE:          return "Ato 2: CG casa à noite";
        case SCENE_ATO2_CG_FRENTE_ESCOLA:       return "Ato 2: CG frente da escola";
        case SCENE_EPILOGUE:                    return "Epílogo";
        case SCENE_ATO3_ABERTURA:               return "Ato 3: abertura";
        case SCENE_ATO3_RESERVA:                return "Ato 3: token de reserva";
        case SCENE_ATO3_OFERTA:                 return "Ato 3: oferta da IA";
        case SCENE_ATO3_IA_LE:                  return "Ato 3: IA lê o papel";
        case SCENE_ATO3_IA_LE_IA_ESCREVEU:      return "Ato 3: IA lê (IA escreveu)";
        case SCENE_ATO3_AJUDA:                  return "Ato 3: tirar uma opção";
        case SCENE_ATO3_SOZINHA:                return "Ato 3: tentar sozinha";
        case SCENE_ATO3_CONCLUSAO:              return "Ato 3: conclusão";
        case SCENE_ATO3_CONCLUSAO_IA_ESCREVEU:  return "Ato 3: conclusão (IA escreveu)";
        case SCENE_ATO3_ETAPA_CONCLUIDA:        return "Ato 3: etapa concluída";
        default:                                return "cena desconhecida";
    }
}

const char *Game_NomeDaTela(GameScreen tela)
{
    switch (tela)
    {
        case SCREEN_MENU:          return "Menu";
        case SCREEN_GAME:          return "História";
        case SCREEN_CREDITS:       return "Créditos";
        case SCREEN_ENDING:        return "Tela final";
        case SCREEN_ROOM:          return "Quarto (Ato 1)";
        case SCREEN_AUDIO:         return "Áudio";
        case SCREEN_SENHA:         return "Senha (Ato 1)";
        case SCREEN_CORRIDA:       return "Corrida (Ato 2)";
        case SCREEN_PAUSE:         return "Pausa";
        case SCREEN_CONFIRM_MENU:  return "Confirmar menu";
        case SCREEN_ENTRE_LINHAS:  return "Entre as Linhas (Ato 3)";
        default:                   return "outra";
    }
}

#endif