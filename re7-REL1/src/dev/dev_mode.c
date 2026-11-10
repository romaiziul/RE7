// dev_mode.c
// Painel do modo desenvolvedor. Compilado so com -DRE7_DEV_MODE (.\build.ps1 -Dev).
// Os comandos chamam a API do game.c (Game_*), que usa as mesmas funcoes
// da campanha. Aqui fica so a interface.
#include "dev_mode.h"

#ifdef RE7_DEV_MODE

#include "raylib.h"
#include "ui.h"
#include "tokens.h"

// Atalhos (funcionam com o painel aberto ou fechado)
#define TECLA_PAINEL       KEY_F10
#define TECLA_PULAR_FALA   KEY_F6
#define TECLA_PROX_ETAPA   KEY_F7
#define TECLA_REINICIAR    KEY_F8

typedef enum
{
    ABA_NAVEGACAO,
    ABA_TOKENS,
    ABA_VARIAVEIS,
    ABA_TESTES,
    ABA_TOTAL
} Aba;

static const char *NOMES_ABAS[ABA_TOTAL] = { "Navegação", "Tokens", "Variáveis", "Testes" };

static bool painelAberto = false;
static Aba abaAtual = ABA_NAVEGACAO;
static bool capturou = false;

static char mensagem[128] = "";
static float mensagemTempo = 0.0f;

static const Rectangle PAINEL = {880, 96, 388, 500};

// Cores de ferramenta (diferentes da interface do jogo)
#define DEV_FUNDO     (Color){30, 30, 30, 240}
#define DEV_BARRA     (Color){0, 122, 204, 255}
#define DEV_BOTAO     (Color){62, 62, 66, 255}
#define DEV_SOBRE     (Color){85, 85, 92, 255}
#define DEV_ATIVO     (Color){14, 99, 156, 255}
#define DEV_TEXTO     (Color){220, 220, 220, 255}
#define DEV_DISCRETO  (Color){150, 150, 150, 255}
#define DEV_OK        (Color){78, 201, 176, 255}
#define DEV_AVISO     (Color){220, 180, 90, 255}


// =====================================================
// AUXILIARES
// =====================================================

// Copia o texto: o TextFormat da raylib reaproveita o mesmo buffer
static void Avisar(const char *texto)
{
    TextCopy(mensagem, TextSubtext(texto, 0, (int)sizeof(mensagem) - 1));
    mensagemTempo = 3.0f;
}

static const char *SimNao(bool valor)
{
    return valor ? "Sim" : "Não";
}

static const char *NomeCampanha(EstadoCampanha c)
{
    switch (c)
    {
        case CAMPANHA_FINAL_BOM:  return "final bom";
        case CAMPANHA_FINAL_RUIM: return "final ruim";
        default:                  return "em andamento";
    }
}

static const char *NomeAto(AtoId ato)
{
    switch (ato)
    {
        case ATO_2:       return "Ato 2";
        case ATO_3:       return "Ato 3";
        case ATO_EPILOGO: return "Epílogo";
        default:          return "Ato 1";
    }
}

// Area de conteudo da aba (abaixo das abas, acima do estado)
static Rectangle Celula(int coluna, int linha, int colunas, float altura, float topo)
{
    const float margem = 12.0f, folga = 8.0f;
    float largura = (PAINEL.width - 2 * margem - (colunas - 1) * folga) / colunas;

    return (Rectangle){
        PAINEL.x + margem + coluna * (largura + folga),
        PAINEL.y + topo + linha * (altura + folga),
        largura,
        altura
    };
}

// Botao do painel: devolve true no clique. "ligado" pinta como selecionado.
// Chamado no Atualizar (para agir) e no Desenhar (com desenhar = true).
static bool Botao(Rectangle r, const char *texto, bool ligado, bool desenhar)
{
    bool sobre = CheckCollisionPointRec(GetMousePosition(), r);

    if (desenhar)
    {
        DrawRectangleRec(r, ligado ? DEV_ATIVO : (sobre ? DEV_SOBRE : DEV_BOTAO));
        DrawRectangleLinesEx(r, 1.0f, ligado ? DEV_BARRA : Fade(WHITE, 0.15f));

        int fs = 17;
        int larg = UiMeasureText(texto, fs);
        UiDrawText(texto, (int)(r.x + (r.width - larg) / 2), (int)(r.y + (r.height - fs) / 2 - 1), fs, DEV_TEXTO);
        return false;
    }

    return sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}


// =====================================================
// CONTEUDO DE CADA ABA
// O mesmo codigo desenha (desenhar = true) e trata os cliques (false),
// para os botoes ficarem sempre no mesmo lugar.
// =====================================================

#define TOPO_CONTEUDO 84.0f

static void AbaNavegacao(GameState *game, bool desenhar)
{
    static const AtoId atos[4] = { ATO_1, ATO_2, ATO_3, ATO_EPILOGO };
    AtoId atual = Game_AtoAtual(game);

    for (int i = 0; i < 4; i++)
    {
        Rectangle r = Celula(i % 2, i / 2, 2, 44, TOPO_CONTEUDO + 26);

        if (Botao(r, NomeAto(atos[i]), atual == atos[i], desenhar))
        {
            Game_IrParaAto(game, atos[i]);
            Avisar(TextFormat("%s carregado.", NomeAto(atos[i])));
        }
    }

    if (desenhar)
    {
        UiDrawText("Ir direto para o início do ato:", (int)PAINEL.x + 12, (int)(PAINEL.y + TOPO_CONTEUDO), 16, DEV_DISCRETO);
        UiDrawText("Tokens e variáveis atuais são mantidos.", (int)PAINEL.x + 12, (int)(PAINEL.y + TOPO_CONTEUDO + 136), 15, DEV_DISCRETO);
    }
}

static void AbaTokens(GameState *game, bool desenhar)
{
    (void)game;

    for (int v = 0; v <= TOKENS_MAX; v++)
    {
        Rectangle r = Celula(v, 0, TOKENS_MAX + 1, 48, TOPO_CONTEUDO + 26);

        if (Botao(r, TextFormat("%d", v), Tokens_Valor() == v, desenhar))
        {
            Tokens_Restaurar(v, Tokens_ReservaUsada());
            Avisar(TextFormat("Barra de tokens em %d.", v));
        }
    }

    if (desenhar)
    {
        UiDrawText("Quantidade de tokens da IA:", (int)PAINEL.x + 12, (int)(PAINEL.y + TOPO_CONTEUDO), 16, DEV_DISCRETO);
        UiDrawText("A barra do jogo atualiza na hora.", (int)PAINEL.x + 12, (int)(PAINEL.y + TOPO_CONTEUDO + 88), 15, DEV_DISCRETO);
        UiDrawText("A reserva fica na aba Variáveis.", (int)PAINEL.x + 12, (int)(PAINEL.y + TOPO_CONTEUDO + 110), 15, DEV_DISCRETO);
    }
}

static void AbaVariaveis(GameState *game, bool desenhar)
{
    float y = TOPO_CONTEUDO;

    // IA escreveu o discurso no Ato 2
    if (desenhar) UiDrawText("IA escreveu o discurso no Ato 2", (int)PAINEL.x + 12, (int)(PAINEL.y + y), 16, DEV_DISCRETO);
    for (int i = 0; i < 2; i++)
    {
        bool valor = (i == 0);
        if (Botao(Celula(i, 0, 2, 34, y + 22), SimNao(valor), game->iaEscreveuDiscurso == valor, desenhar))
        {
            game->iaEscreveuDiscurso = valor;
            Avisar(TextFormat("IA escreveu o discurso: %s.", SimNao(valor)));
        }
    }

    // Reserva de token ja utilizada
    y += 70;
    if (desenhar) UiDrawText("Reserva de token já utilizada", (int)PAINEL.x + 12, (int)(PAINEL.y + y), 16, DEV_DISCRETO);
    for (int i = 0; i < 2; i++)
    {
        bool valor = (i == 0);
        if (Botao(Celula(i, 0, 2, 34, y + 22), SimNao(valor), Tokens_ReservaUsada() == valor, desenhar))
        {
            Tokens_Restaurar(Tokens_Valor(), valor);
            Avisar(TextFormat("Reserva usada: %s.", SimNao(valor)));
        }
    }

    // Estado da campanha
    y += 70;
    if (desenhar) UiDrawText("Estado da campanha", (int)PAINEL.x + 12, (int)(PAINEL.y + y), 16, DEV_DISCRETO);
    static const EstadoCampanha estados[3] = { CAMPANHA_EM_ANDAMENTO, CAMPANHA_FINAL_BOM, CAMPANHA_FINAL_RUIM };
    static const char *rotulos[3] = { "Andamento", "Final bom", "Final ruim" };
    for (int i = 0; i < 3; i++)
    {
        if (Botao(Celula(i, 0, 3, 34, y + 22), rotulos[i], game->campanha == estados[i], desenhar))
        {
            game->campanha = estados[i];
            Avisar(TextFormat("Campanha: %s.", NomeCampanha(estados[i])));
        }
    }

    if (desenhar)
        UiDrawText("Para ir à tela final, use a aba Testes.", (int)PAINEL.x + 12, (int)(PAINEL.y + y + 66), 15, DEV_DISCRETO);
}

typedef enum
{
    TESTE_PULAR_FALA,
    TESTE_PROX_ETAPA,
    TESTE_REINICIAR_ATO,
    TESTE_REINICIAR_CAMPANHA,
    TESTE_FINAL_BOM,
    TESTE_FINAL_RUIM,
    TESTE_TOTAL
} Teste;

static void ExecutarTeste(GameState *game, Teste teste)
{
    switch (teste)
    {
        case TESTE_PULAR_FALA:
            Avisar(Game_PularFala(game) ? "Fala pulada." : "Há uma escolha na tela: escolha uma opção.");
            break;

        case TESTE_PROX_ETAPA:
            Avisar(Game_AvancarEtapa(game) ? "Avançou para a próxima etapa." : "Há uma escolha na tela: escolha uma opção.");
            break;

        case TESTE_REINICIAR_ATO:
        {
            AtoId ato = Game_AtoAtual(game);
            Game_ReiniciarAto(game);
            Avisar(TextFormat("%s reiniciado.", NomeAto(ato)));
            break;
        }

        case TESTE_REINICIAR_CAMPANHA:
            Game_ReiniciarCampanha(game);
            Avisar("Campanha reiniciada (barra em 3).");
            break;

        case TESTE_FINAL_BOM:
            Game_ForcarFinal(game, CAMPANHA_FINAL_BOM);
            Avisar("Final bom.");
            break;

        case TESTE_FINAL_RUIM:
            Game_ForcarFinal(game, CAMPANHA_FINAL_RUIM);
            Avisar("Final ruim.");
            break;

        default:
            break;
    }
}

static void AbaTestes(GameState *game, bool desenhar)
{
    static const char *rotulos[TESTE_TOTAL] =
    {
        "Pular diálogo (F6)", "Próxima etapa (F7)",
        "Reiniciar ato (F8)", "Reiniciar campanha",
        "Forçar final bom",   "Forçar final ruim"
    };

    for (int i = 0; i < TESTE_TOTAL; i++)
    {
        if (Botao(Celula(i % 2, i / 2, 2, 40, TOPO_CONTEUDO), rotulos[i], false, desenhar))
            ExecutarTeste(game, (Teste)i);
    }

    if (desenhar)
        UiDrawText("Próxima etapa: fim da cena ou do minijogo.", (int)PAINEL.x + 12, (int)(PAINEL.y + TOPO_CONTEUDO + 148), 15, DEV_DISCRETO);
}

static void ConteudoDaAba(GameState *game, bool desenhar)
{
    switch (abaAtual)
    {
        case ABA_NAVEGACAO: AbaNavegacao(game, desenhar); break;
        case ABA_TOKENS:    AbaTokens(game, desenhar);    break;
        case ABA_VARIAVEIS: AbaVariaveis(game, desenhar); break;
        case ABA_TESTES:    AbaTestes(game, desenhar);    break;
        default: break;
    }
}

static Rectangle AreaAba(int i)
{
    float largura = (PAINEL.width - 24) / ABA_TOTAL;
    return (Rectangle){PAINEL.x + 12 + i * largura, PAINEL.y + 42, largura - 4, 30};
}


// =====================================================
// ATUALIZAR
// =====================================================

void DevMode_Atualizar(GameState *game)
{
    capturou = false;

    if (mensagemTempo > 0.0f) mensagemTempo -= GetFrameTime();

    if (IsKeyPressed(TECLA_PAINEL)) painelAberto = !painelAberto;

    // atalhos rapidos
    if (IsKeyPressed(TECLA_PULAR_FALA)) ExecutarTeste(game, TESTE_PULAR_FALA);
    if (IsKeyPressed(TECLA_PROX_ETAPA)) ExecutarTeste(game, TESTE_PROX_ETAPA);
    if (IsKeyPressed(TECLA_REINICIAR))  ExecutarTeste(game, TESTE_REINICIAR_ATO);

    if (!painelAberto) return;

    // cliques dentro do painel nao chegam ao jogo
    if (CheckCollisionPointRec(GetMousePosition(), PAINEL) &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        capturou = true;

    for (int i = 0; i < ABA_TOTAL; i++)
    {
        if (Botao(AreaAba(i), NOMES_ABAS[i], (int)abaAtual == i, false))
            abaAtual = (Aba)i;
    }

    ConteudoDaAba(game, false);
}

bool DevMode_CapturouEntrada(void)
{
    return capturou;
}


// =====================================================
// DESENHAR
// =====================================================

static void DesenharIndicador(void)
{
    const char *texto = painelAberto ? "DEV MODE  ·  F10 fecha o painel" : "DEV MODE  ·  F10";
    int fs = 14;
    int larg = UiMeasureText(texto, fs);
    Rectangle r = {(1280 - larg) / 2.0f - 10, 0, larg + 20.0f, 20};

    DrawRectangleRec(r, Fade((Color){180, 40, 40, 255}, 0.85f));
    UiDrawText(texto, (int)r.x + 10, 2, fs, WHITE);
}

void DevMode_Desenhar(const GameState *gameConst)
{
    DesenharIndicador();

    if (!painelAberto) return;

    // o desenho nao muda o estado (desenhar = true)
    GameState *game = (GameState *)gameConst;

    DrawRectangleRec(PAINEL, DEV_FUNDO);
    DrawRectangleLinesEx(PAINEL, 1.0f, DEV_BARRA);

    // barra de titulo
    DrawRectangle((int)PAINEL.x, (int)PAINEL.y, (int)PAINEL.width, 34, DEV_BARRA);
    UiDrawTextEmphasis("DEVELOPER MODE", (int)PAINEL.x + 12, (int)PAINEL.y + 7, 19, WHITE);
    UiDrawText("F10", (int)(PAINEL.x + PAINEL.width - 40), (int)PAINEL.y + 9, 16, Fade(WHITE, 0.8f));

    for (int i = 0; i < ABA_TOTAL; i++)
        Botao(AreaAba(i), NOMES_ABAS[i], (int)abaAtual == i, true);

    ConteudoDaAba(game, true);

    // estado atual
    float y = PAINEL.y + PAINEL.height - 150;
    int x = (int)PAINEL.x + 12;
    int fs = 15;

    DrawLine(x, (int)y - 8, (int)(PAINEL.x + PAINEL.width) - 12, (int)y - 8, Fade(WHITE, 0.15f));

    Scene scene = GetScene(game->currentScene);

    UiDrawText(TextFormat("Tela: %s", Game_NomeDaTela(game->currentScreen)), x, (int)y, fs, DEV_TEXTO);
    UiDrawText(TextFormat("Cena: %s (fala %d/%d)", Game_NomeDaCena(game->currentScene),
                          game->dialogueIndex + 1, scene.dialogueCount),
               x, (int)y + 20, fs, DEV_TEXTO);
    UiDrawText(TextFormat("%s  ·  Tokens: %d/%d  ·  Reserva usada: %s",
                          NomeAto(Game_AtoAtual(game)), Tokens_Valor(), TOKENS_MAX, SimNao(Tokens_ReservaUsada())),
               x, (int)y + 40, fs, DEV_TEXTO);
    UiDrawText(TextFormat("IA escreveu: %s  ·  Campanha: %s",
                          SimNao(game->iaEscreveuDiscurso), NomeCampanha(game->campanha)),
               x, (int)y + 60, fs, DEV_TEXTO);
    UiDrawText("Save de teste: save_dev.dat (o save.dat não é usado)", x, (int)y + 84, 14, DEV_DISCRETO);

    if (mensagem[0] != '\0' && mensagemTempo > 0.0f)
        UiDrawText(mensagem, x, (int)y + 112, 16, Fade(mensagemTempo > 0.5f ? DEV_OK : DEV_AVISO, mensagemTempo > 0.5f ? 1.0f : mensagemTempo * 2.0f));
}

#endif
