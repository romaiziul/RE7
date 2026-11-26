// Teste do controlador REAL da tela, sem abrir janela. So a fronteira da
// raylib e substituida: teclas, fila de texto, relogio e sons.
#include <stdio.h>
#include <string.h>
#define GetFrameTime Test_GetFrameTime
#define IsWindowFocused Test_IsWindowFocused
#define GetRandomValue Test_GetRandomValue
#define GetCharPressed Test_GetCharPressed
#define IsKeyPressed Test_IsKeyPressed
#define IsKeyDown Test_IsKeyDown
#define IsMouseButtonPressed Test_IsMouseButtonPressed
#define IsButtonClicked Test_IsButtonClicked
#define AudioManager_Play Test_AudioManager_Play
#include "../src/atos/ato3/entre_linhas.c"
static int falhas, verificacoes, pressed[512], held[512], texto[32], pos, total;
static float dt = 0.016f;
static int clique = -1;
static bool foco = true;
#define CHECK(x) do { verificacoes++; if(!(x)) {falhas++; printf("FALHOU %d: %s\n",__LINE__,#x);} } while(0)
float GetFrameTime(void) { return dt; }
bool IsWindowFocused(void) { return foco; }
int GetRandomValue(int min,int max) { (void)max; return min; }
int GetCharPressed(void) { return pos<total?texto[pos++]:0; }
bool IsKeyPressed(int k) { return pressed[k]; }
bool IsKeyDown(int k) { return held[k]; }
bool IsMouseButtonPressed(int b) { (void)b; return false; }
bool IsButtonClicked(Rectangle r) { return clique>=0 && (int)r.x==(int)AreaBotao(clique, Ato3_Etapa(desafio.modo, desafio.etapa)->total).x; }
void AudioManager_Play(EfeitoSonoro som) { (void)som; }

static void quadro(int key, int down, const char *s) {
    memset(pressed,0,sizeof pressed); memset(held,0,sizeof held);
    if(key>=0) pressed[key]=1;
    if(down>=0) held[down]=1;
    pos=total=0;
    while(*s) texto[total++]=(unsigned char)*s++;
    UpdateAto3EntreLinhas(); clique=-1;
}
static void iniciar(Ato3Ajuda modo) {
    InitAto3EntreLinhas(modo); dt=0.3f; quadro(-1,-1,""); dt=0.016f;
}
int main(void) {
    iniciar(ATO3_AJUDA_SOZINHA);
    quadro(KEY_W,KEY_W,"w");
    CHECK(oratoria.digitando && oratoria.progresso==0);
    quadro(-1,-1,"p"); // solta W e comeca a palavra no mesmo quadro
    CHECK(oratoria.progresso==1 && oratoria.erros==0);
    quadro(-1,-1,"ublico");
    CHECK(desafio.fase==EL_FASE_ACERTO);

    iniciar(ATO3_AJUDA_SOZINHA);
    clique=1; quadro(-1,-1,"");
    quadro(-1,-1,"p");
    CHECK(oratoria.progresso==1);

    iniciar(ATO3_AJUDA_SOZINHA);
    quadro(KEY_W,KEY_W,"w");
    quadro(-1,KEY_W,"w");
    CHECK(oratoria.progresso==0 && oratoria.erros==0);
    quadro(-1,KEY_D,"p"); // so a tecla usada para selecionar exige soltura
    CHECK(oratoria.progresso==1);

    iniciar(ATO3_AJUDA_TIRAR_OPCAO);
    quadro(KEY_W,KEY_W,"w"); CHECK(!oratoria.digitando);
    quadro(KEY_RIGHT,KEY_RIGHT,""); CHECK(oratoria.digitando && oratoria.opcao==1);
    quadro(-1,-1,"publico"); CHECK(desafio.fase==EL_FASE_ACERTO);

    iniciar(ATO3_AJUDA_SOZINHA);
    quadro(KEY_UP,KEY_UP,""); quadro(-1,-1,"");
    float tempo=oratoria.restante;
    foco=false; dt=20; quadro(-1,-1,"p"); foco=true; dt=0.016f;
    CHECK(oratoria.restante==tempo && oratoria.progresso==0);
    printf("%d verificacoes de entrada, %d falhas\n",verificacoes,falhas);
    return falhas?1:0;
}
