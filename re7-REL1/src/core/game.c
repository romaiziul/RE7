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
// SAVE / LOAD (arquivo de texto: "cena fala tokens reserva")
// =====================================================

#define SAVE_FILE "save.dat"

static bool SaveGame(GameState *game)
{
    return SaveFileText(
        SAVE_FILE,
        (char *)TextFormat(
            "%d %d %d %d",
            (int)game->currentScene,
            game->dialogueIndex,
            Tokens_Valor(),
            Tokens_ReservaUsada() ? 1 : 0
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
    int lidos = sscanf(text, "%d %d %d %d", &scene, &index, &tokens, &reserva);

    UnloadFileText(text);

    // O save anterior tem tres campos e ainda pode ser carregado.
    if (lidos != 3 && lidos != 4) return false;

    if (scene < 0 || scene >= SCENE_COUNT) return false;

    if (tokens < 0 || tokens > TOKENS_MAX) tokens = TOKENS_MAX;

    Scene s = GetScene((SceneID)scene);

    if (index < 0 || index >= s.dialogueCount) index = 0;

    game->currentScene = (SceneID)scene;
    game->dialogueIndex = index;
    game->choice = 0;

    Tokens_Restaurar(tokens, reserva == 1);
    BarraTokens_Reiniciar();

    return true;
}


// =====================================================
// EFEITO DE VIAGEM NO TEMPO (flash) E PERSONAGEM
// =====================================================

#define FLASH_DURACAO 1.4f

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
static void DrawPersonagem(Texture2D tex, float altura, float yTopo, float xDireita)
{
    float largura = altura * (float)tex.width / (float)tex.height;

    Rectangle destino = { xDireita - largura, yTopo, largura, altura };

    DrawTexturePro(
        tex,
        (Rectangle){ 0, 0, (float)tex.width, (float)tex.height },
        destino,
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

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
                case SCENE_ATO2_CG_ENSAIO:
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
    FUNDO_CG_ENSAIO
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

        case SCENE_ATO2_CORREDOR:
        case SCENE_ATO2_POSTER:         return (VisualCena){ FUNDO_CORREDOR,       2007, true  };

        case SCENE_ATO2_LANHOUSE:
        case SCENE_ATO2_IA_ESCREVE:
        case SCENE_ATO2_UMA_IDEIA:
        case SCENE_ATO2_FECHAR:         return (VisualCena){ FUNDO_LANHOUSE,       2007, false };

        case SCENE_ATO2_DISCURSO:
        case SCENE_ATO2_ENSAIO:         return (VisualCena){ FUNDO_LANHOUSE,       2007, true  };

        case SCENE_ATO2_CG_ESCREVEM:    return (VisualCena){ FUNDO_CG_ESCREVEM,    0,    false };
        case SCENE_ATO2_CG_PROFESSORA:  return (VisualCena){ FUNDO_CG_PROFESSORA,  0,    false };
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

// Cenario provisorio da lan house (a arte definitiva ainda sera feita)
static void DesenharLanHouse(void)
{
    float tempo = (float)GetTime();

    DrawRectangleGradientV(0, 0, 1280, 720, (Color){22, 20, 58, 255}, (Color){66, 36, 100, 255});

    // fileira de monitores ao fundo
    for (int i = 0; i < 6; i++)
    {
        float x = 70.0f + i * 200.0f;
        float y = 200.0f;
        float brilho = 0.30f + 0.10f * sinf(tempo * 2.0f + i);

        DrawRectangleRounded((Rectangle){ x, y, 150, 110 }, 0.12f, 6, (Color){ 18, 18, 34, 255 });
        DrawRectangle((int)x + 10, (int)y + 10, 130, 80, Fade((Color){ 80, 200, 255, 255 }, brilho));
        DrawRectangle((int)x + 62, (int)y + 110, 26, 22, (Color){ 18, 18, 34, 255 });
    }

    // bancada
    DrawRectangle(0, 360, 1280, 360, (Color){ 40, 26, 72, 255 });
    DrawRectangle(0, 360, 1280, 6, (Color){ 255, 105, 200, 255 });
}


// =====================================================
// ESCOLHAS (aparecem sobre a propria tela de dialogo)
// =====================================================

typedef enum
{
    ESCOLHA_NENHUMA = 0,
    ESCOLHA_ANO,        // Ato 1: em que ano voce esta?
    ESCOLHA_AMIGO,      // Ato 2: dar uma chance ao amigo
    ESCOLHA_IA          // Ato 2: o que responder a IA na lan house
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
    }
};

// Opcao errada: pisca em vermelho por um instante e a escolha continua
static float escolhaErro = 0.0f;
static int escolhaErroOpcao = -1;

// Area clicavel de cada opcao
static Rectangle AreaOpcao(int escolha, int indice)
{
    float topo = (escolha == ESCOLHA_IA) ? 250.0f : 230.0f;

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

    if (game->choice == ESCOLHA_IA)
    {
        DesenharPopupMSN();
        UiDrawText(e->pergunta, 340, 175, 28, (Color){ 29, 38, 96, 255 });
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

        const char *texto = TextFormat("%d   %s", i + 1, e->opcoes[i]);
        int larguraTexto = UiMeasureText(texto, 26);

        UiDrawText(texto, (int)(r.x + (r.width - larguraTexto) / 2.0f), (int)r.y + 20, 26, WHITE);
    }
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
            if (opcao == 0)                         // dar uma chance ao amigo
            {
                game->choice = ESCOLHA_NENHUMA;
                ChangeScene(game, SCENE_ATO2_POSTER);
            }
            else                                    // ignorar: o amigo alcanca e a escolha volta
            {
                escolhaErro = 0.5f;
                escolhaErroOpcao = opcao;
                return;
            }
            break;

        case ESCOLHA_IA:
            game->choice = ESCOLHA_NENHUMA;

            if (opcao == 0)                         // IA escreve o discurso: zera
            {
                Tokens_Zerar();
                ChangeScene(game, SCENE_ATO2_IA_ESCREVE);
            }
            else if (opcao == 1)                    // so uma ideia: -1
            {
                Tokens_Gastar();
                ChangeScene(game, SCENE_ATO2_UMA_IDEIA);
            }
            else                                    // fechar a janela: +1
            {
                Tokens_Recuperar();
                ChangeScene(game, SCENE_ATO2_FECHAR);
            }
            break;

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

        case SCENE_ATO2_POSTER:
            // Desafio 2: corrida de obstaculos.
            game->dialogueIndex = scene.dialogueCount - 1;
            SaveGame(game);
            InitAto2Corrida();
            game->currentScreen = SCREEN_CORRIDA;
            return;

        case SCENE_ATO2_LANHOUSE:
            game->dialogueIndex = scene.dialogueCount - 1;
            game->choice = ESCOLHA_IA;
            escolhaErro = 0.0f;
            return;

        case SCENE_ATO3_ABERTURA:
            if (Tokens_UsarReserva())
                ChangeScene(game, SCENE_ATO3_RESERVA);
            else
                ChangeScene(game, SCENE_ATO3_OFERTA);
            return;

        case SCENE_EPILOGUE:
            // fim da historia: volta ao inicio (para jogar de novo)
            // e mostra a tela final
            ChangeScene(game, SCENE_PRESENT);
            SaveGame(game);
            game->currentScreen = SCREEN_ENDING;
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

    Tokens_Reiniciar();
    BarraTokens_Reiniciar();
    Senha_ComecarJogo();
}


// =====================================================
// TROCAR DE CENA
// =====================================================

void ChangeScene(
    GameState *game,
    SceneID nextScene
)
{
    game->currentScene = nextScene;

    game->dialogueIndex = 0;
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

    game->choice = 0;
    game->senhaNoQuarto = false;

    Tokens_Reiniciar();                             // barra volta a 3
    BarraTokens_Reiniciar();
    Senha_ComecarJogo();

    SaveGame(game);                                 // zera o save
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
           tela == SCREEN_SENHA || tela == SCREEN_CORRIDA;
}

static Rectangle AreaBotaoMenu(GameScreen tela)
{
    const float largura = 104.0f;
    const float margem = 14.0f;

    if (tela == SCREEN_ROOM)
        return (Rectangle){GetScreenWidth() - largura - margem, margem, largura, 40};

    // Na senha, o cabecalho ocupa a parte de cima da tela.
    float y = (tela == SCREEN_SENHA) ? 666.0f : margem;
    return (Rectangle){margem, y, largura, 40};
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
        if (IsButtonClicked(cancelarRetorno))
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

    if (IsButtonClicked(fecharPausa))
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
        {
            game->currentScreen = SCREEN_GAME;
            ChangeScene(game, SCENE_ATO1_IA_A);
            SaveGame(game);
        }

        return;
    }


    // -------------------------------------------------
    // CORRIDA DE OBSTACULOS (Ato 2)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CORRIDA)
    {
        MinigameResult resultado = UpdateAto2Corrida();

        if (resultado == MINIGAME_VENCEU)      // chegou ao fim do corredor
        {
            game->currentScreen = SCREEN_GAME;
            ChangeScene(game, SCENE_ATO2_LANHOUSE);
            SaveGame(game);
        }

        return;
    }


    // -------------------------------------------------
    // EXPLORACAO DO QUARTO
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ROOM)
    {
        RoomResult resultadoQuarto = UpdateRoom();

        if (resultadoQuarto == ROOM_CONCLUIDO)
        {
            // Fim da exploracao: o PC esta bloqueado
            game->currentScreen = SCREEN_GAME;
            ChangeScene(game, SCENE_2007_PC);
            SaveGame(game);
        }

        return;
    }


    // -------------------------------------------------
    // JOGO (HISTORIA)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_GAME)
    {
        // Enquanto ha uma escolha na tela, so as opcoes respondem
        if (game->choice != ESCOLHA_NENHUMA)
        {
            AtualizarEscolha(game);

            return;
        }

        Scene scene = GetScene(game->currentScene);

       if (IsKeyPressed(KEY_SPACE) ||
    IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            game->dialogueIndex++;

            if (game->dialogueIndex >= scene.dialogueCount)
            {
                FimDaCena(game, scene);

                if (game->currentScreen != SCREEN_GAME || game->choice != ESCOLHA_NENHUMA)
                    return;
            }

            SaveGame(game);                         // salva a cada fala
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

void DrawPauseControls(GameState *game)
{
    if (PodePausar(game->currentScreen))
        DesenharBotaoPausa(AreaBotaoMenu(game->currentScreen), "Menu", 20);
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

    DrawCenteredText("Clique no X para retomar a partida", 583, 18, Fade(WHITE, 0.7f));
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
        DesenharFundo();

        DrawCenteredText("FIM", 150, 90, WHITE);
        DrawCenteredText("Lia aprendeu a confiar na propria voz.", 280, 30, SKYBLUE);
        DrawCenteredText("Obrigada por jogar Re:7!", 330, 28, WHITE);

        DrawCenteredText("ENTER - jogar novamente", 440, 28, WHITE);
        DrawCenteredText("M - voltar ao menu", 485, 28, WHITE);
        DrawCenteredText("ESC - sair do jogo", 530, 28, WHITE);

        if (avisoInvalido > 0.0f)
        {
            DrawCenteredText(
                "Opcao invalida! Use ENTER, M ou ESC.",
                600, 24,
                Fade((Color){255, 105, 200, 255}, fminf(1.0f, avisoInvalido))
            );
        }

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

        if (game->currentScene == SCENE_ATO3_ABERTURA ||
            game->currentScene == SCENE_ATO3_RESERVA ||
            game->currentScene == SCENE_ATO3_OFERTA)
        {
            Ato3_DesenharCenario();

            Color telaPC = Tokens_IAOffline()
                ? (Color){25, 28, 35, 255}
                : (Color){74, 190, 135, 255};
            DrawRectangle(684, 293, 42, 28, telaPC);

            if (game->currentScene != SCENE_ATO3_ABERTURA ||
                game->dialogueIndex >= 1)
            {
                Ato3_DesenharPapel();
            }

            DrawDialogueBox(dialogue.speaker, dialogue.text);
            BarraTokens_Desenhar(990, 20);

            UiDrawText(
                "CLIQUE ou ESPACO para continuar",
                760, 690, 18, Fade(WHITE, 0.8f)
            );

            DesenharAvisoSom();
            return;
        }

        VisualCena visual = VisualDaCena(game->currentScene);

        // Fundo

        switch (visual.fundo)
        {
            case FUNDO_2026:          DesenharImagemTela(assets->bgPresent);     break;
            case FUNDO_2007:          DesenharImagemTela(assets->bg2007);        break;
            case FUNDO_CORREDOR:      DesenharImagemTela(assets->bgCorredor);    break;
            case FUNDO_LANHOUSE:      DesenharLanHouse();                        break;
            case FUNDO_CG_TELA:       DesenharImagemTela(assets->cgTela2026);    break;
            case FUNDO_CG_ESCREVEM:   DesenharImagemTela(assets->cgEscrevem);    break;
            case FUNDO_CG_PROFESSORA: DesenharImagemTela(assets->cgProfessora);  break;
            case FUNDO_CG_ENSAIO:     DesenharImagemTela(assets->cgEnsaio);      break;
        }

        // Personagens: o amigo a esquerda, a Lia a direita
        // (altura, posicao Y do topo, borda direita)

        if (visual.amigo)
        {
            Pose poseAmigo = PoseDe(&scene, game->dialogueIndex, "Amigo");

            DrawPersonagem(
                assets->amigo[poseAmigo == POSE_FELIZ ? 1 : 0],
                900.0f, -101.0f, 610.0f
            );
        }

        if (visual.lia != 0)
        {
            Pose poseLia = PoseDe(&scene, game->dialogueIndex, "Lia");

            Texture2D texLia = TexturaLia(assets, visual.lia, poseLia);

            if (visual.lia == 2007)
                DrawPersonagem(texLia, 900.0f, -101.0f, 1250.0f);
            else
                DrawPersonagem(texLia, 820.0f, -7.0f, 1250.0f);
        }

        // Poster do concurso (Ato 2): aparece enquanto o amigo fala dele
        if (game->currentScene == SCENE_ATO2_POSTER && game->dialogueIndex <= 1)
        {
            float altura = 400.0f;
            float largura = altura * (float)assets->poster.width / (float)assets->poster.height;
            Rectangle area = { (1280.0f - largura) / 2.0f, 60.0f, largura, altura };

            float pulso = 0.5f + 0.5f * sinf((float)GetTime() * 4.0f);

            DrawRectangleRounded(
                (Rectangle){ area.x - 8, area.y - 8, area.width + 16, area.height + 16 },
                0.04f, 8, Fade((Color){ 255, 105, 200, 255 }, 0.35f + 0.4f * pulso)
            );

            DrawTexturePro(
                assets->poster,
                (Rectangle){ 0, 0, (float)assets->poster.width, (float)assets->poster.height },
                area, (Vector2){ 0, 0 }, 0.0f, WHITE
            );
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

        // Exibe o aviso de som por cima da tela atual
        DesenharAvisoSom();
    }


    // Para desenvolvimento:

    // DrawDebugGrid();

    // DrawMousePosition();
}
