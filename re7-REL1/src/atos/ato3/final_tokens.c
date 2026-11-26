#include "final_tokens.h"
#include "raylib.h"
#include "ui.h"
#include "tela.h"
#include <math.h>

static int momentoTokens;
static float tempoTokens;
static const Color MARFIM_TOKENS={249,235,213,255};
static const Color AZUL_TOKENS={167,190,223,255};

void FinalTokens_Iniciar(void) { momentoTokens=0; tempoTokens=0; }

static float MinimoMomentoTokens(void) {
    // A descoberta tem tempo para completar a passagem de feliz para triste.
    return momentoTokens==1 ? 3.0f : 0.55f;
}

static float TristezaTokens(void) {
    if(momentoTokens==0) return 0;
    if(momentoTokens>1) return 1;
    float t=fminf(1,fmaxf(0,(tempoTokens-0.5f)/1.9f));
    return t*t*(3-2*t);
}

static Rectangle BotaoTokens(int i) {
    return (Rectangle){80,415+i*67,430,53};
}

static void PassoTokens(float dt, bool avancar) {
    if(isfinite(dt) && dt>0) tempoTokens+=dt;
    if(momentoTokens<3 && tempoTokens>=MinimoMomentoTokens() && avancar) {
        momentoTokens++;
        tempoTokens=0;
    }
}

FinalTokensAcao FinalTokens_Atualizar(void) {
    if(!IsWindowFocused()) return FINAL_TOKENS_CONTINUA;
    bool avancar=IsKeyPressed(KEY_ENTER)||IsKeyPressed(KEY_SPACE)||
                 IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if(momentoTokens<3) {
        PassoTokens(GetFrameTime(),avancar);
        while(GetKeyPressed()) {}
        return FINAL_TOKENS_CONTINUA;
    }
    PassoTokens(GetFrameTime(),false);
    if(tempoTokens<MinimoMomentoTokens()) return FINAL_TOKENS_CONTINUA;
    if(IsKeyPressed(KEY_ENTER)||IsButtonClicked(BotaoTokens(0))) return FINAL_TOKENS_REJOGAR;
    if(IsKeyPressed(KEY_M)||IsButtonClicked(BotaoTokens(1))) return FINAL_TOKENS_MENU;
    if(IsKeyPressed(KEY_ESCAPE)||IsButtonClicked(BotaoTokens(2))) return FINAL_TOKENS_SAIR;
    return FINAL_TOKENS_CONTINUA;
}

static void FundoTokens(Texture2D tex) {
    ClearBackground((Color){23,24,38,255});
    if(!tex.id || tex.width<=0 || tex.height<=0) return;
    float escala=fmaxf((float)TELA_LARGURA/tex.width,(float)TELA_ALTURA/tex.height);
    Rectangle src={(tex.width-TELA_LARGURA/escala)/2,(tex.height-TELA_ALTURA/escala)/2,
                   TELA_LARGURA/escala,TELA_ALTURA/escala};
    DrawTexturePro(tex,src,(Rectangle){0,0,TELA_LARGURA,TELA_ALTURA},(Vector2){0,0},0,WHITE);
}

static void RetratoTokens(Texture2D tex, float alpha) {
    if(!tex.id || tex.height<=0 || alpha<=0) return;
    float h=624,w=h*tex.width/tex.height;
    DrawTexturePro(tex,(Rectangle){0,0,tex.width,tex.height},
        (Rectangle){944-w/2,TELA_ALTURA-h,w,h},(Vector2){0,0},0,Fade(WHITE,alpha));
}

static void TextoTokens(const char *s,int y,int fs,Color cor) {
    UiDrawText(s,80,y,fs,cor);
}

static void AvisoRetorno(void) {
    Rectangle r={80,425,510,136};
    DrawRectangleRounded(r,0.08f,8,(Color){25,31,48,240});
    DrawRectangleRoundedLines(r,0.08f,8,Fade(AZUL_TOKENS,0.5f));
    UiDrawText("RE:7 Messenger  /  conexão encerrada",98,440,17,AZUL_TOKENS);
    UiDrawTextEmphasis("RETORNO INDISPONÍVEL",98,472,24,MARFIM_TOKENS);
    UiDrawText("Cargas restantes",98,515,18,AZUL_TOKENS);
    UiDrawTextEmphasis("0 / 3",474,511,24,(Color){222,154,170,255});
    for(int i=0;i<3;i++) {
        Rectangle carga={310+i*47,517,37,12};
        DrawRectangleRoundedLines(carga,0.2f,6,Fade(AZUL_TOKENS,0.55f));
    }
}

void FinalTokens_Desenhar(const Assets *assets) {
    float triste=TristezaTokens();
    FundoTokens(momentoTokens<2?assets->ato3Auditorio:assets->ato3FinalSemRetorno);
    if(momentoTokens<2) {
        // O palco vai perdendo a luz quente junto com a expressao da Lia.
        DrawRectangle(0,0,TELA_LARGURA,TELA_ALTURA,
            Fade((Color){23,26,47,255},0.16f+0.28f*triste));
        Texture2D tristeTex=assets->lia07SemRetorno;
        if(!tristeTex.id) tristeTex=assets->lia07Envergonhada;
        // Uma expressao desaparece na sombra antes de a outra surgir.
        // Nao sobrepoe os olhos e a boca dos dois retratos.
        float visibilidade=0.08f+0.92f*fabsf(2*triste-1);
        RetratoTokens(triste<0.5f?assets->lia07ConfiantePalco:tristeTex,visibilidade);
    }
    DrawRectangleGradientH(0,0,TELA_LARGURA,TELA_ALTURA,
        Fade((Color){17,20,34,255},momentoTokens==3?0.97f:0.91f),
        Fade((Color){17,20,34,255},momentoTokens==3?0.30f:0.04f));
    TextoTokens("RE:7  /  ATO 3",55,18,AZUL_TOKENS);
    UiDrawText("F11  tela cheia",1090,55,16,Fade(MARFIM_TOKENS,0.7f));
    switch(momentoTokens) {
        case 0:
            TextoTokens("DEPOIS DOS APLAUSOS",202,18,(Color){230,197,145,255});
            UiDrawTextEmphasis("Eu consegui!",80,246,42,MARFIM_TOKENS);
            TextoTokens("Lia termina o discurso e ouve os aplausos.",322,23,MARFIM_TOKENS);
            TextoTokens("O nervosismo dá lugar a um sorriso.",359,23,MARFIM_TOKENS);
            TextoTokens("\"Agora eu posso voltar para casa.\"",438,25,MARFIM_TOKENS);
            break;
        case 1:
            TextoTokens("A CONEXÃO SE APAGA",187,18,AZUL_TOKENS);
            UiDrawTextEmphasis("Mas eu terminei...",80,230,37,MARFIM_TOKENS);
            TextoTokens("Lia procura a IA na tela do computador.",302,23,MARFIM_TOKENS);
            TextoTokens("O contador está vazio. Não há resposta.",337,23,MARFIM_TOKENS);
            AvisoRetorno();
            break;
        case 2:
            TextoTokens("2007",187,18,AZUL_TOKENS);
            UiDrawTextEmphasis("O futuro não vem.",80,230,37,MARFIM_TOKENS);
            TextoTokens("Ganhar o desafio não devolveu as cargas.",306,23,MARFIM_TOKENS);
            TextoTokens("Sem elas, a IA não pode levá-la de volta.",341,23,MARFIM_TOKENS);
            TextoTokens("A alegria do palco se transforma em saudade.",411,22,MARFIM_TOKENS);
            TextoTokens("\"Eu só queria voltar para casa.\"",476,25,MARFIM_TOKENS);
            break;
        default: {
            UiDrawTextEmphasis("Sem caminho de volta",80,210,36,MARFIM_TOKENS);
            TextoTokens("Lia encontrou a própria voz.",281,23,MARFIM_TOKENS);
            TextoTokens("Mas não restou energia para voltar ao futuro.",316,23,MARFIM_TOKENS);
            TextoTokens("Sua história continua em 2007.",351,23,AZUL_TOKENS);
            const char *nomes[]={"Jogar novamente","Voltar ao menu","Sair do jogo"};
            const char *teclas[]={"ENTER","M","ESC"};
            for(int i=0;i<3;i++) {
                Rectangle r=BotaoTokens(i);
                bool hover=tempoTokens>=MinimoMomentoTokens()&&CheckCollisionPointRec(GetMousePosition(),r);
                DrawRectangleRounded(r,0.14f,8,hover?(Color){57,72,105,245}:(Color){29,35,53,225});
                DrawRectangleRoundedLines(r,0.14f,8,Fade(AZUL_TOKENS,hover?0.95f:0.35f));
                UiDrawText(nomes[i],r.x+20,r.y+14,22,MARFIM_TOKENS);
                UiDrawText(teclas[i],r.x+r.width-75,r.y+18,16,AZUL_TOKENS);
            }
            TextoTokens("Um novo jogo começa do início da história.",640,17,Fade(MARFIM_TOKENS,0.65f));
            break;
        }
    }
    if(momentoTokens<3 && tempoTokens>=MinimoMomentoTokens()) {
        TextoTokens("CLIQUE, ESPAÇO ou ENTER para continuar",637,18,Fade(MARFIM_TOKENS,0.8f));
        for(int i=0;i<4;i++) DrawCircle(87+i*23,683,4,i==momentoTokens?AZUL_TOKENS:Fade(AZUL_TOKENS,0.3f));
    }
    // Mantem a continuidade visual entre o sorriso e a descoberta.
    if(momentoTokens!=1) {
        float a=1-fminf(tempoTokens/0.4f,1);
        if(a>0) DrawRectangle(0,0,TELA_LARGURA,TELA_ALTURA,Fade(BLACK,a));
    }
}
