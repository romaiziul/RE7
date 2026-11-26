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
static bool MockBotao(Rectangle r) { return clique>=0 && r.y==400+clique*67; }
#define GetFrameTime MockDt
#define IsKeyPressed MockTecla
#define IsMouseButtonPressed MockMouse
#define IsWindowFocused MockFoco
#define GetKeyPressed MockFila
#define IsButtonClicked MockBotao
#include "../src/atos/ato3/final_discurso.c"
#undef GetFrameTime
#undef IsKeyPressed
#undef IsMouseButtonPressed
#undef IsWindowFocused
#undef GetKeyPressed
#undef IsButtonClicked

static void Capturar(const Assets *assets, RenderTexture2D quadro, const char *nome) {
    BeginTextureMode(quadro);
    FinalDiscurso_Desenhar(assets);
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
        Rectangle r=BotaoFinal(i);
        SetMouseOffset((int)-mx,(int)-my); SetMouseScale(1/s,1/s);
        SetMousePosition((int)(mx+(r.x+r.width/2)*s),(int)(my+(r.y+r.height/2)*s));
        assert(CheckCollisionPointRec(GetMousePosition(),r));
    }
    printf("Cena %d: render e mouse %dx%d OK\n",momento,w,h);
}

int main(void) {
    FinalDiscurso_Iniciar();
    relogio=0.1f; tecla=KEY_ENTER;
    assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_CONTINUA && momento==0);
    foco=false; relogio=5;
    FinalDiscurso_Atualizar(); assert(momento==0 && tempo<0.2f);
    foco=true; relogio=0.6f; tecla=0;
    FinalDiscurso_Atualizar(); assert(momento==0);
    tecla=KEY_SPACE; relogio=0;
    FinalDiscurso_Atualizar(); assert(momento==1 && tempo==0);
    FinalDiscurso_Atualizar(); assert(momento==1);
    tecla=0; avancarMouse=true; relogio=0.6f;
    assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_CONTINUA && momento==2);
    avancarMouse=false; tecla=KEY_ENTER; relogio=0;
    assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_CONTINUA);
    relogio=0.6f;
    assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_REJOGAR);
    tecla=KEY_M; assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_MENU);
    tecla=KEY_ESCAPE; assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_SAIR);
    tecla=0;
    for(clique=0;clique<3;clique++)
        assert(FinalDiscurso_Atualizar()==FINAL_DISCURSO_REJOGAR+clique);
    clique=-1;
    FinalDiscurso_Iniciar(); assert(momento==0 && tempo==0);
    puts("Sequencia, foco, bloqueio, reinicio e acoes por teclado/mouse: OK");

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_WINDOW_RESIZABLE);
    InitWindow(1280,720,"Teste final do discurso"); UiLoadFonts();
    Assets assets={0}; LoadAssets(&assets);
    assert(assets.ato3FinalBastidores.id && assets.lia07Envergonhada.id);
    RenderTexture2D quadro=LoadRenderTexture(1280,720);
    SetTextureFilter(quadro.texture,TEXTURE_FILTER_BILINEAR);
    tempo=1;
    Capturar(&assets,quadro,"validacao/final-palco.png");
    ToggleBorderlessWindowed(); assert(IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE));
    momento=1; Capturar(&assets,quadro,"validacao/final-bastidores.png");
    momento=2; Capturar(&assets,quadro,"validacao/final-cartao.png");
    ToggleBorderlessWindowed(); SetWindowSize(1000,800);
    Capturar(&assets,quadro,"validacao/final-janela.png");
    UnloadRenderTexture(quadro); UnloadAssets(&assets); UiUnloadFonts(); CloseWindow();
    puts("Assets, tres cenas, tela cheia e cliques alinhados: OK");
    return 0;
}
