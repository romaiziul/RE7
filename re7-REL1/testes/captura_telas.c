// captura_telas.c
// Gera imagens (PNG) das telas do Ato 3 e do modo desenvolvedor, sem
// precisar jogar: abre uma janela escondida, coloca o jogo em cada estado
// usando o proprio codigo do jogo e salva o quadro.
//
// Compilar (Windows, w64devkit), dentro de re7-REL1 (inclui todos os .c de
// src, menos main.c, escolha.c, entre_linhas.c e dev_mode.c, que entram
// por #include abaixo):
//   $f = Get-ChildItem src -Recurse -Filter *.c | ? { $_.Name -notin 'main.c','escolha.c','entre_linhas.c','dev_mode.c' } | % FullName
//   $i = @('-Isrc') + (Get-ChildItem src -Recurse -Directory | % { '-I' + $_.FullName })
//   gcc testes/captura_telas.c @f @i -DRE7_DEV_MODE -IC:/raylib/raylib/src -LC:/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm -o captura_telas.exe
// Rodar (dentro de re7-REL1, por causa dos assets):
//   .\captura_telas.exe <pasta-de-saida>

#include "raylib.h"

#include "game.h"
#include "assets.h"
#include "ui.h"
#include "tokens.h"
#include "barra_tokens.h"

// estado interno do desafio e do painel, para montar cada tela
#include "../src/atos/ato3/entre_linhas.c"
#include "../src/dev/dev_mode.c"

static RenderTexture2D quadro;
static Assets assets;
static GameState game;
static const char *pasta = ".";

static void Desenhar(int quadros)
{
    for (int i = 0; i < quadros; i++)
    {
        BarraTokens_Atualizar();

        BeginTextureMode(quadro);
        ClearBackground(BLACK);
        DrawGame(&game, &assets);
        DrawPauseControls(&game);
        DevMode_Desenhar(&game);
        EndTextureMode();

        BeginDrawing();     // so para o relogio da raylib andar
        EndDrawing();
    }
}

static void Salvar(const char *nome)
{
    Image img = LoadImageFromTexture(quadro.texture);
    ImageFlipVertical(&img);
    ExportImage(img, TextFormat("%s/%s.png", pasta, nome));
    UnloadImage(img);
}

// Desafio: passa o tempo (animacoes e bloqueio)
static void Passar(float segundos)
{
    EntreLinhas_Atualizar(&desafio, segundos);
}

int main(int argc, char **argv)
{
    if (argc > 1) pasta = argv[1];

    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(1280, 720, "captura");
    SetTargetFPS(60);
    UiLoadFonts();
    LoadAssets(&assets);
    quadro = LoadRenderTexture(1280, 720);

    InitGame(&game);

    // ---- Ato 3: abertura (barra em 2) ----
    Tokens_Restaurar(2, false);
    Game_IrParaAto(&game, ATO_3);
    game.dialogueIndex = 1;
    Desenhar(5);
    Salvar("01_ato3_abertura");

    // ---- Barra em 0: token de reserva (o PC pisca) ----
    Tokens_Restaurar(0, false);
    Game_IrParaAto(&game, ATO_3);
    game.dialogueIndex = 1;
    Game_AvancarEtapa(&game);                       // fim da abertura
    Desenhar(12);
    Salvar("02_ato3_reserva");

    // ---- Oferta e escolha no MSN ----
    Game_AvancarEtapa(&game);                       // reserva -> oferta
    Game_AvancarEtapa(&game);                       // fim da oferta: escolha
    Desenhar(3);
    Salvar("03_ato3_escolha");

    // ---- Desafio com ajuda (tirar uma opcao errada) ----
    game.choice = 0;
    Tokens_Restaurar(1, true);
    Ato3_AplicarAjuda(ATO3_AJUDA_TIRAR_OPCAO);
    ChangeScene(&game, SCENE_ATO3_AJUDA);
    Desenhar(2);
    Salvar("04_ato3_cena_ajuda");
    Game_AvancarEtapa(&game);                       // fim da cena: abre o desafio
    Passar(0.4f);
    Desenhar(3);
    Salvar("05_desafio_com_ajuda");

    // erro na palavra 1: a Lia gagueja
    EntreLinhas_Responder(&desafio, 0);
    tremorErro = 0.15f;
    Desenhar(2);
    Salvar("06_desafio_erro_lia");

    // o amigo sopra: a palavra aparece
    Passar(0.3f);
    EntreLinhas_Avancar(&desafio);
    Passar(0.6f);
    Desenhar(2);
    Salvar("07_desafio_erro_amigo");

    // acerto na palavra 2 (no meio da animacao)
    Passar(0.3f);
    EntreLinhas_Avancar(&desafio);
    Passar(0.3f);
    EntreLinhas_Responder(&desafio, 0);
    Passar(0.25f);
    Desenhar(2);
    Salvar("08_desafio_acerto");

    // palavra 3 e conclusao
    Passar(1.0f);
    Passar(0.3f);
    EntreLinhas_Responder(&desafio, 1);
    Passar(1.0f);
    Passar(1.4f);
    Desenhar(3);
    Salvar("09_desafio_conclusao");

    // ---- Desafio sozinha (3 alternativas) ----
    Tokens_Restaurar(3, true);
    InitAto3EntreLinhas(ATO3_AJUDA_SOZINHA);
    Passar(0.4f);
    Desenhar(3);
    Salvar("10_desafio_sozinha");

    // ---- Conclusao: a camera vai para a Lia, plateia aplaude ----
    game.currentScreen = SCREEN_GAME;
    game.iaEscreveuDiscurso = false;
    ChangeScene(&game, SCENE_ATO3_CONCLUSAO);
    Desenhar(80);
    Salvar("11_ato3_conclusao");

    game.iaEscreveuDiscurso = true;
    ChangeScene(&game, SCENE_ATO3_CONCLUSAO_IA_ESCREVEU);
    Desenhar(80);
    Salvar("12_ato3_conclusao_ia_escreveu");

    // ---- Cenario 1: a IA le o papel ----
    Tokens_Restaurar(0, true);
    ChangeScene(&game, SCENE_ATO3_IA_LE_IA_ESCREVEU);
    game.dialogueIndex = GetScene(game.currentScene).dialogueCount - 1;
    Desenhar(5);
    Salvar("13_ato3_ia_le");

    // ---- Finais ----
    Game_ForcarFinal(&game, CAMPANHA_FINAL_RUIM);
    Desenhar(3);
    Salvar("14_final_ruim_ia_leu");

    game.motivoFinalRuim = FINAL_RUIM_SEM_TOKENS;
    Desenhar(3);
    Salvar("15_final_ruim_sem_tokens");

    Game_ForcarFinal(&game, CAMPANHA_FINAL_BOM);
    Desenhar(3);
    Salvar("16_final_bom");

    // ---- Painel do modo desenvolvedor (as quatro abas) ----
    Tokens_Restaurar(2, false);
    Game_IrParaAto(&game, ATO_3);
    game.dialogueIndex = 1;
    painelAberto = true;

    static const char *nomes[ABA_TOTAL] = { "17_dev_navegacao", "18_dev_tokens", "19_dev_variaveis", "20_dev_testes" };
    for (int a = 0; a < ABA_TOTAL; a++)
    {
        abaAtual = (Aba)a;
        Avisar("Ato 3 carregado.");
        Desenhar(2);
        Salvar(nomes[a]);
    }

    UnloadRenderTexture(quadro);
    UnloadAssets(&assets);
    BarraTokens_Descarregar();
    UiUnloadFonts();
    CloseWindow();
    return 0;
}
