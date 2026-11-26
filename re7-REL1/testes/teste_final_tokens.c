#include "raylib.h"
#include <assert.h>
#include <stdio.h>
static float relogio;
static int tecla, clique=-1;
static bool foco=true, avancarMouse;
static float MockDt(void) { return relogio; }
static bool MockTecla(int k) { return tecla==k; }
static bool MockMouse(int b) { (void)b; return avancarMouse; }
static bool MockFoco(void) { return foco; }
static int MockFila(void) { return 0; }
static bool MockBotao(Rectangle r) { return clique>=0 && r.y==415+clique*67; }
#define GetFrameTime MockDt
#define IsKeyPressed MockTecla
#define IsMouseButtonPressed MockMouse
#define IsWindowFocused MockFoco
#define GetKeyPressed MockFila
#define IsButtonClicked MockBotao
#include "../src/atos/ato3/final_tokens.c"
#undef GetFrameTime
#undef IsKeyPressed
#undef IsMouseButtonPressed
#undef IsWindowFocused
#undef GetKeyPressed
#undef IsButtonClicked

static void Capturar(const Assets *assets, RenderTexture2D quadro, const char *nome) {
    BeginTextureMode(quadro);
    FinalTokens_Desenhar(assets);
    EndTextureMode();
    int w=GetScreenWidth(),h=GetScreenHeight();
    RenderTexture2D saida=LoadRenderTexture(w,h);
    float s=fminf(w/1280.0f,h/720.0f), mx=(w-1280*s)/2,my=(h-720*s)/2;
    BeginTextureMode(saida); ClearBackground(BLACK);
    DrawTexturePro(quadro.texture,(Rectangle){0,0,1280,-720},
        (Rectangle){mx,my,1280*s,720*s},(Vector2){0,0},0,WHITE);
    EndTextureMode();
    Image img=LoadImageFromTexture(saida.texture); ImageFlipVertical(&img);
    assert(ExportImage(img,nome)); UnloadImage(img); UnloadRenderTexture(saida);
    for(int i=0;i<3;i++) {
        Rectangle r=BotaoTokens(i);
        SetMouseOffset((int)-mx,(int)-my); SetMouseScale(1/s,1/s);
        SetMousePosition((int)(mx+(r.x+r.width/2)*s),(int)(my+(r.y+r.height/2)*s));
        assert(CheckCollisionPointRec(GetMousePosition(),r));
    }
    printf("Cena %d: render e mouse %dx%d OK\n",momentoTokens,w,h);
}

int main(void) {
    FinalTokens_Iniciar();
    relogio=0.1f; tecla=KEY_ENTER;
    assert(FinalTokens_Atualizar()==FINAL_TOKENS_CONTINUA && momentoTokens==0);
    foco=false; relogio=5; FinalTokens_Atualizar();
    assert(momentoTokens==0 && tempoTokens<0.2f);
    foco=true; relogio=0.6f; tecla=0; FinalTokens_Atualizar();
    assert(momentoTokens==0 && TristezaTokens()==0);
    tecla=KEY_SPACE; relogio=0; FinalTokens_Atualizar();
    assert(momentoTokens==1 && tempoTokens==0 && TristezaTokens()==0);
    // Cliques nao podem saltar a mudanca emocional antes dos tres segundos.
    tecla=0; avancarMouse=true; relogio=1.45f; FinalTokens_Atualizar();
    assert(momentoTokens==1 && TristezaTokens()>0.4f && TristezaTokens()<0.6f);
    relogio=1.0f; FinalTokens_Atualizar();
    assert(momentoTokens==1 && TristezaTokens()==1);
    relogio=0.6f; FinalTokens_Atualizar();
    assert(momentoTokens==2 && tempoTokens==0);
    relogio=0.6f; FinalTokens_Atualizar(); assert(momentoTokens==3);
    avancarMouse=false; tecla=KEY_ENTER; relogio=0;
    assert(FinalTokens_Atualizar()==FINAL_TOKENS_CONTINUA);
    relogio=0.6f; assert(FinalTokens_Atualizar()==FINAL_TOKENS_REJOGAR);
    tecla=KEY_M; assert(FinalTokens_Atualizar()==FINAL_TOKENS_MENU);
    tecla=KEY_ESCAPE; assert(FinalTokens_Atualizar()==FINAL_TOKENS_SAIR);
    tecla=KEY_F11; assert(FinalTokens_Atualizar()==FINAL_TOKENS_CONTINUA);
    tecla=0;
    for(clique=0;clique<3;clique++) assert(FinalTokens_Atualizar()==FINAL_TOKENS_REJOGAR+clique);
    clique=-1; FinalTokens_Iniciar(); assert(momentoTokens==0 && tempoTokens==0);
    puts("Sequencia, transicao emocional, foco, trava e botoes: OK");

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_RESIZABLE);
    InitWindow(1280,720,"Teste final sem cargas"); UiLoadFonts();
    Assets assets={0}; LoadAssets(&assets);
    assert(assets.ato3FinalSemRetorno.id && assets.lia07SemRetorno.id);
    RenderTexture2D quadro=LoadRenderTexture(1280,720);
    SetTextureFilter(quadro.texture,TEXTURE_FILTER_BILINEAR);
    tempoTokens=1;
    Capturar(&assets,quadro,"validacao/tokens-feliz.png");
    ToggleBorderlessWindowed(); assert(IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE));
    momentoTokens=1; tempoTokens=1.45f;
    Capturar(&assets,quadro,"validacao/tokens-transicao.png");
    tempoTokens=3.2f;
    Capturar(&assets,quadro,"validacao/tokens-triste.png");
    momentoTokens=2; Capturar(&assets,quadro,"validacao/tokens-sem-retorno.png");
    momentoTokens=3; Capturar(&assets,quadro,"validacao/tokens-cartao.png");
    ToggleBorderlessWindowed(); SetWindowSize(1000,800);
    Capturar(&assets,quadro,"validacao/tokens-janela.png");
    UnloadRenderTexture(quadro); UnloadAssets(&assets); UiUnloadFonts(); CloseWindow();
    puts("Assets, quatro momentos, tela cheia e mouse: OK");
    return 0;
}
