/*
 * Ato 2 - Desafio 2: corrida de obstaculos no corredor da escola.
 *
 * Controles: ESPACO, W ou seta para cima para pular. Clique em Menu para pausar.
 *
 * Regras:
 *   - O corredor passa por baixo da Lia durante DURACAO_SEGUNDOS.
 *   - Obstaculos (sorteados entre os 6 desenhos): mp3 player, celular flip, tenis vermelho,
 *     CD, pilha de mangas e fichario.
 *   - Bater num obstaculo: a Lia tropeca, perde PENALIDADE_SEGUNDOS de progresso
 *     e fica um tempo invulneravel. Nao existe game over.
 *   - Perto do fim, os obstaculos acabam e o amigo aparece parado no fim do corredor.
 *     O fundo vai parando, a Lia chega ate ele e para. Depois a tela escurece
 *     rapidinho e o minijogo devolve MINIGAME_VENCEU (o encontro e a fala ficam no game.c).
 *
 * Desenha em 800x450 e a camera amplia para a janela do jogo (1280x720),
 * igual ao minigame.c antigo.
 */

#include "ato2_corrida.h"
#include "audio_manager.h"

#include "raylib.h"
#include <stdbool.h>
#include <time.h>


/* ---------- Ajustes (mexa aqui para acertar o visual e a dificuldade) ---------- */

#define LARGURA            800
#define ALTURA             450
#define CHAO_Y             380      /* linha dos pes da Lia (y) */

#define DURACAO_SEGUNDOS   28.0f    /* tempo ate chegar ao fim do corredor */
#define PENALIDADE_SEGUNDOS 2.0f    /* progresso perdido ao tropecar */
#define TEMPO_TROPECO      0.9f
#define TEMPO_INVULNERAVEL 1.6f
#define TEMPO_FADE_FINAL   0.6f

#define JOG_X_INICIAL      120.0f   /* centro da Lia (x) quando a corrida comeca */
#define JOG_LARGURA        50.0f    /* caixa de colisao (menor que o desenho) */
#define JOG_ALTURA         110.0f

#define GRAVIDADE          2200.0f
#define FORCA_PULO         780.0f
#define VEL_INICIAL        380.0f
#define VEL_MAXIMA         700.0f
#define ACELERACAO         12.0f

#define MAX_OBSTACULOS     6

/* Chegada ao amigo (fim da corrida) */
#define SEM_OBSTACULOS_ANTES 3.5f   /* segundos antes do fim em que param de nascer obstaculos */
#define TEMPO_CHEGADA        1.4f   /* segundos antes do fim em que o amigo aparece */
#define AMIGO_ALTURA         165.0f /* altura do amigo na tela (800x450) */
#define AMIGO_X_PARADO       610.0f /* onde o amigo para (borda esquerda do desenho) */
#define DISTANCIA_PARADA     95.0f  /* distancia final entre a Lia e o amigo (centro a centro) */
#define VEL_LIA_CHEGADA      260.0f /* velocidade da Lia indo ate o amigo */
#define DESACELERACAO_FUNDO  800.0f /* o fundo vai parando */
#define TEMPO_PARADA         0.8f   /* pausa com a Lia parada antes de escurecer */

#define INICIO_MUSICA_SEGUNDOS 1.2f /* onde a trilha da corrida comeca (pula o silencio inicial) */

/* Escala dos desenhos da Lia (a imagem original e grande; aqui ela fica ~150 px de altura) */
#define ESCALA_CORRIDA     0.22f
#define ESCALA_PULO        0.165f   /* o desenho do pulo/tropeco veio em escala maior */
#define ESCALA_TROPECO     0.165f
#define FPS_CORRIDA        10.0f


/* ---------- Recortes dos desenhos (area visivel, sem a margem transparente) ---------- */

#define FRAME_LARGURA      560.0f   /* os 4 quadros de corrida tem 560x724 */
#define FRAME_ALTURA       724.0f
#define FRAME_PE_Y         686.0f   /* y dos pes dentro do quadro */

static const Rectangle RECORTE_PULO    = { 452.0f,  16.0f,  624.0f,  910.0f };
static const Rectangle RECORTE_TROPECO = { 212.0f,  13.0f, 1134.0f,  998.0f };
static const Rectangle RECORTE_AMIGO   = { 379.0f,   8.0f,  266.0f, 1406.0f };  /* amigo parado, de lado */

typedef struct {
    const char *arquivo;
    Rectangle recorte;   /* area visivel do desenho */
    float altura;        /* altura na tela */
} TipoObstaculo;

#define NUM_TIPOS 6

static const TipoObstaculo TIPOS[NUM_TIPOS] =
{
    { "assets/ato2/ato2_corrida_obstaculo_01.png", {  48.0f,  49.0f, 1451.0f,  926.0f },  70.0f },  /* mp3 player */
    { "assets/ato2/ato2_corrida_obstaculo_02.png", {  35.0f,  18.0f, 1259.0f, 1157.0f },  84.0f },  /* celular flip */
    { "assets/ato2/ato2_corrida_obstaculo_03.png", {  39.0f,  56.0f, 1459.0f,  925.0f },  72.0f },  /* tenis vermelho */
    { "assets/ato2/ato2_corrida_obstaculo_04.png", {  64.0f,  31.0f, 1430.0f,  961.0f },  66.0f },  /* CD */
    { "assets/ato2/ato2_corrida_obstaculo_05.png", {  95.0f,  19.0f, 1372.0f,  991.0f },  66.0f },  /* pilha de mangas */
    { "assets/ato2/ato2_corrida_obstaculo_06.png", {  63.0f,  60.0f, 1037.0f, 1218.0f },  98.0f }   /* fichario */
};


/* ---------- Estado ---------- */

typedef struct {
    Rectangle corpo;   /* caixa de colisao */
    Rectangle visual;  /* onde o desenho aparece */
    int tipo;
    bool ativo;
} Obstaculo;

static Texture2D texFundo;
static Texture2D texCorrida[4];
static Texture2D texPulo;
static Texture2D texTropeco;
static Texture2D texObstaculo[NUM_TIPOS];
static Texture2D texAmigo;
static bool carregado = false;

static float jogX;              /* x do centro da Lia */
static float jogadorY;          /* y dos pes da Lia */
static float velY;
static bool noChao;

static Obstaculo obstaculos[MAX_OBSTACULOS];
static float velocidade;
static float tempoSpawn;
static float progresso;         /* segundos de corredor percorridos */
static float deslocamentoFundo; /* quanto o fundo ja andou, em pixels da tela */
static float tempoAnim;
static float tempoTropeco;
static float tempoInvulneravel;
static bool chegando;           /* o amigo apareceu: sem obstaculos, o fundo vai parando */
static bool parada;             /* a Lia chegou ate o amigo */
static float tempoParada;
static float amigoX;            /* borda esquerda do amigo na tela */
static bool terminando;         /* fade final */
static float tempoFade;


/* ---------- Carregar e liberar imagens ---------- */

static void CarregarImagens(void)
{
    if (carregado) return;

    texFundo = LoadTexture("assets/ato2/ato2_corrida_bg.png");

    texCorrida[0] = LoadTexture("assets/ato2/ato2_corrida_lia_01.png");
    texCorrida[1] = LoadTexture("assets/ato2/ato2_corrida_lia_02.png");
    texCorrida[2] = LoadTexture("assets/ato2/ato2_corrida_lia_03.png");
    texCorrida[3] = LoadTexture("assets/ato2/ato2_corrida_lia_04.png");

    texPulo    = LoadTexture("assets/ato2/ato2_corrida_lia_05.png");
    texTropeco = LoadTexture("assets/ato2/ato2_corrida_tropeco.png");
    texAmigo   = LoadTexture("assets/ato2/ato2_corrida_amigo.png");

    for (int i = 0; i < NUM_TIPOS; i++) texObstaculo[i] = LoadTexture(TIPOS[i].arquivo);

    carregado = true;
}

void UnloadAto2Corrida(void)
{
    if (!carregado) return;

    UnloadTexture(texFundo);
    for (int i = 0; i < 4; i++) UnloadTexture(texCorrida[i]);
    UnloadTexture(texPulo);
    UnloadTexture(texTropeco);
    UnloadTexture(texAmigo);
    for (int i = 0; i < NUM_TIPOS; i++) UnloadTexture(texObstaculo[i]);

    carregado = false;
}


/* ---------- Logica ---------- */

static void CriarObstaculo(void)
{
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) continue;

        int tipo = GetRandomValue(0, NUM_TIPOS - 1);
        const TipoObstaculo *t = &TIPOS[tipo];

        float altura  = t->altura;
        float largura = altura * t->recorte.width / t->recorte.height;
        float x = LARGURA + 20.0f;

        obstaculos[i].tipo = tipo;
        obstaculos[i].ativo = true;
        obstaculos[i].visual = (Rectangle){ x, CHAO_Y - altura, largura, altura };

        /* caixa de colisao um pouco menor que o desenho */
        obstaculos[i].corpo = (Rectangle){
            x + largura * 0.12f, CHAO_Y - altura * 0.85f,
            largura * 0.76f, altura * 0.85f
        };
        return;
    }
}

void InitAto2Corrida(void)
{
    CarregarImagens();

    /* a faixa tem ~1,2 s de silencio/fade-in no comeco: pula para a musica entrar logo */
    AudioManager_PlayMusicFrom(MUSICA_AVENTURA, INICIO_MUSICA_SEGUNDOS);
    SetRandomSeed((unsigned int)time(NULL));

    jogX = JOG_X_INICIAL;
    jogadorY = CHAO_Y;
    velY = 0.0f;
    noChao = true;

    for (int i = 0; i < MAX_OBSTACULOS; i++) obstaculos[i].ativo = false;

    velocidade = VEL_INICIAL;
    tempoSpawn = 1.4f;
    progresso = 0.0f;
    deslocamentoFundo = 0.0f;
    tempoAnim = 0.0f;
    tempoTropeco = 0.0f;
    tempoInvulneravel = 0.0f;
    chegando = false;
    parada = false;
    tempoParada = 0.0f;
    amigoX = LARGURA + 40.0f;
    terminando = false;
    tempoFade = 0.0f;
}

MinigameResult UpdateAto2Corrida(void)
{
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;

    /* fim: escurece e devolve a vitoria */
    if (terminando) {
        tempoFade += dt;
        if (tempoFade >= TEMPO_FADE_FINAL) return MINIGAME_VENCEU;
    }

    bool tropecando = tempoTropeco > 0.0f;

    /* pulo */
    bool pediuPulo = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    if (pediuPulo && noChao && !tropecando && !chegando) {
        velY = -FORCA_PULO;
        noChao = false;
    }

    velY += GRAVIDADE * dt;
    jogadorY += velY * dt;
    if (jogadorY >= CHAO_Y) {
        jogadorY = CHAO_Y;
        velY = 0.0f;
        noChao = true;
    }

    /* velocidade, fundo e progresso */
    if (tropecando) {
        tempoTropeco -= dt;
    } else {
        velocidade += ACELERACAO * dt;
        if (velocidade > VEL_MAXIMA) velocidade = VEL_MAXIMA;
    }
    if (tempoInvulneravel > 0.0f) tempoInvulneravel -= dt;

    deslocamentoFundo += velocidade * dt;
    tempoAnim += dt;

    if (!chegando) {
        progresso += dt;
        if (progresso >= DURACAO_SEGUNDOS - TEMPO_CHEGADA) {
            /* o amigo aparece no fim do corredor; some qualquer obstaculo a frente da Lia */
            chegando = true;
            amigoX = LARGURA + 40.0f;
            for (int i = 0; i < MAX_OBSTACULOS; i++)
                if (obstaculos[i].ativo && obstaculos[i].visual.x > jogX) obstaculos[i].ativo = false;
        }
    } else if (!terminando) {
        progresso += dt;
        if (progresso > DURACAO_SEGUNDOS) progresso = DURACAO_SEGUNDOS;
    }

    /* chegada: o amigo entra com o fundo, para, e a Lia vai ate ele */
    if (chegando && !terminando) {
        if (amigoX > AMIGO_X_PARADO) {
            amigoX -= velocidade * dt;
            if (amigoX < AMIGO_X_PARADO) amigoX = AMIGO_X_PARADO;
        } else {
            /* o fundo vai parando e a Lia segue ate o amigo */
            velocidade -= DESACELERACAO_FUNDO * dt;
            if (velocidade < 0.0f) velocidade = 0.0f;

            if (!parada && !tropecando) {
                float amigoCentro = amigoX + (RECORTE_AMIGO.width * (AMIGO_ALTURA / RECORTE_AMIGO.height)) / 2.0f;
                jogX += VEL_LIA_CHEGADA * dt;
                if (amigoCentro - jogX <= DISTANCIA_PARADA) {
                    jogX = amigoCentro - DISTANCIA_PARADA;
                    parada = true;
                }
            }
        }

        if (parada) {
            tempoParada += dt;
            if (tempoParada >= TEMPO_PARADA) {
                terminando = true;
                tempoFade = 0.0f;
            }
        }
    }

    /* novos obstaculos (nao nascem perto do fim do corredor) */
    if (!chegando && progresso < DURACAO_SEGUNDOS - SEM_OBSTACULOS_ANTES) {
        tempoSpawn -= dt;
        if (tempoSpawn <= 0.0f) {
            CriarObstaculo();
            tempoSpawn = 1.1f + (float)GetRandomValue(0, 100) / 100.0f;
        }
    }

    /* caixa de colisao da Lia */
    Rectangle hitbox = { jogX - JOG_LARGURA / 2.0f, jogadorY - JOG_ALTURA, JOG_LARGURA, JOG_ALTURA };

    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        Obstaculo *o = &obstaculos[i];
        if (!o->ativo) continue;

        float passo = velocidade * dt;
        o->visual.x -= passo;
        o->corpo.x  -= passo;

        if (o->visual.x + o->visual.width < 0.0f) {
            o->ativo = false;
            continue;
        }

        if (!chegando && tempoInvulneravel <= 0.0f && CheckCollisionRecs(hitbox, o->corpo)) {
            o->ativo = false;
            tempoTropeco = TEMPO_TROPECO;
            tempoInvulneravel = TEMPO_INVULNERAVEL;
            velocidade = VEL_INICIAL;
            progresso -= PENALIDADE_SEGUNDOS;
            if (progresso < 0.0f) progresso = 0.0f;
        }
    }

    return MINIGAME_CONTINUA;
}


/* ---------- Desenho ---------- */

static void DesenharFundo(void)
{
    float escala = (float)ALTURA / (float)texFundo.height;
    float largura = (float)texFundo.width * escala;

    float x = -(float)((int)deslocamentoFundo % (int)largura);

    Rectangle origem = { 0.0f, 0.0f, (float)texFundo.width, (float)texFundo.height };

    /* duas copias lado a lado: quando uma sai, a outra ja esta entrando */
    DrawTexturePro(texFundo, origem, (Rectangle){ x, 0.0f, largura, (float)ALTURA },
                   (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    DrawTexturePro(texFundo, origem, (Rectangle){ x + largura, 0.0f, largura, (float)ALTURA },
                   (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

static void DesenharObstaculos(void)
{
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        const Obstaculo *o = &obstaculos[i];
        if (!o->ativo) continue;

        DrawTexturePro(texObstaculo[o->tipo], TIPOS[o->tipo].recorte, o->visual,
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

static void DesenharAmigo(void)
{
    if (!chegando) return;

    float h = AMIGO_ALTURA;
    float w = RECORTE_AMIGO.width * (h / RECORTE_AMIGO.height);

    DrawTexturePro(texAmigo, RECORTE_AMIGO,
                   (Rectangle){ amigoX, CHAO_Y - h, w, h },
                   (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

static void DesenharLia(void)
{
    /* piscando enquanto esta invulneravel (menos no tropeco, que ja e um desenho proprio) */
    bool tropecando = tempoTropeco > 0.0f;
    if (!tropecando && tempoInvulneravel > 0.0f && ((int)(tempoInvulneravel * 12.0f) % 2 == 0)) return;

    if (tropecando) {
        float w = RECORTE_TROPECO.width * ESCALA_TROPECO;
        float h = RECORTE_TROPECO.height * ESCALA_TROPECO;
        DrawTexturePro(texTropeco, RECORTE_TROPECO,
                       (Rectangle){ jogX - w / 2.0f, jogadorY - h, w, h },
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    } else if (!noChao) {
        float w = RECORTE_PULO.width * ESCALA_PULO;
        float h = RECORTE_PULO.height * ESCALA_PULO;
        DrawTexturePro(texPulo, RECORTE_PULO,
                       (Rectangle){ jogX - w / 2.0f, jogadorY - h, w, h },
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    } else {
        int quadro = parada ? 0 : (int)(tempoAnim * FPS_CORRIDA) % 4;   /* parada: quadro fixo */
        float w = FRAME_LARGURA * ESCALA_CORRIDA;
        float h = FRAME_ALTURA * ESCALA_CORRIDA;
        DrawTexturePro(texCorrida[quadro], (Rectangle){ 0.0f, 0.0f, FRAME_LARGURA, FRAME_ALTURA },
                       (Rectangle){ jogX - w / 2.0f, jogadorY - FRAME_PE_Y * ESCALA_CORRIDA, w, h },
                       (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

/* Barra de progresso do corredor (sem texto) */
static void DesenharBarra(void)
{
    float fracao = progresso / DURACAO_SEGUNDOS;
    if (fracao > 1.0f) fracao = 1.0f;

    Rectangle fundo = { 250.0f, 14.0f, 300.0f, 12.0f };
    DrawRectangleRec(fundo, Fade(BLACK, 0.45f));
    DrawRectangleRec((Rectangle){ fundo.x, fundo.y, fundo.width * fracao, fundo.height }, (Color){ 255, 120, 190, 255 });
    DrawRectangleLinesEx(fundo, 2.0f, WHITE);
}

void DrawAto2Corrida(void)
{
    float zoomX = (float)GetScreenWidth() / (float)LARGURA;
    float zoomY = (float)GetScreenHeight() / (float)ALTURA;

    Camera2D camera = { 0 };
    camera.zoom = (zoomX < zoomY) ? zoomX : zoomY;

    BeginMode2D(camera);

    ClearBackground(BLACK);
    DesenharFundo();
    DesenharObstaculos();
    DesenharAmigo();
    DesenharLia();
    DesenharBarra();

    if (terminando) {
        float alfa = tempoFade / TEMPO_FADE_FINAL;
        if (alfa > 1.0f) alfa = 1.0f;
        DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, alfa));
    }

    EndMode2D();
}
