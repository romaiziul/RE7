#include "final_discurso.h"
#include "raylib.h"
#include "ui.h"
#include "tela.h"
#include <math.h>

// Dois momentos narrativos, seguidos pelo cartao final. O jogador controla
// o ritmo; o bloqueio impede que o clique do minijogo pule a primeira cena.
static int momento;
static float tempo;
static const float BLOQUEIO = 0.55f;
static const Color MARFIM = {249, 235, 213, 255};
static const Color ROSA = {218, 161, 177, 255};

void FinalDiscurso_Iniciar(void) { momento = 0; tempo = 0.0f; }

static Rectangle BotaoFinal(int i) {
    return (Rectangle){80, 400 + i * 67, 430, 53};
}

// Isolado da entrada para testar bloqueio e sequencia sem abrir janela.
static void AvancarMomento(float dt, bool avancar) {
    if (isfinite(dt) && dt > 0) tempo += dt;
    if (momento < 2 && tempo >= BLOQUEIO && avancar) {
        momento++;
        tempo = 0;
    }
}

FinalDiscursoAcao FinalDiscurso_Atualizar(void) {
    if (!IsWindowFocused()) return FINAL_DISCURSO_CONTINUA;
    bool continuar = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (momento < 2) {
        AvancarMomento(GetFrameTime(), continuar);
        // Evita que o ENTER usado para avancar seja reaproveitado pelo menu.
        while (GetKeyPressed()) {}
        return FINAL_DISCURSO_CONTINUA;
    }
    AvancarMomento(GetFrameTime(), false);
    if (tempo < BLOQUEIO) return FINAL_DISCURSO_CONTINUA;
    if (IsKeyPressed(KEY_ENTER) || IsButtonClicked(BotaoFinal(0))) return FINAL_DISCURSO_REJOGAR;
    if (IsKeyPressed(KEY_M) || IsButtonClicked(BotaoFinal(1))) return FINAL_DISCURSO_MENU;
    if (IsKeyPressed(KEY_ESCAPE) || IsButtonClicked(BotaoFinal(2))) return FINAL_DISCURSO_SAIR;
    return FINAL_DISCURSO_CONTINUA;
}

static void Fundo(Texture2D tex) {
    if (!tex.id || tex.width <= 0 || tex.height <= 0) {
        ClearBackground((Color){30, 22, 35, 255});
        return;
    }
    // Cover centralizado: nao estica a arte quando ela tem outra proporcao.
    float escala = fmaxf((float)TELA_LARGURA/tex.width, (float)TELA_ALTURA/tex.height);
    Rectangle src = {(tex.width-TELA_LARGURA/escala)/2,
                     (tex.height-TELA_ALTURA/escala)/2,
                     TELA_LARGURA/escala,TELA_ALTURA/escala};
    DrawTexturePro(tex,src,(Rectangle){0,0,TELA_LARGURA,TELA_ALTURA},(Vector2){0,0},0,WHITE);
}

static void Linha(const char *s, int y, int fs, Color cor) {
    UiDrawText(s,80,y,fs,cor);
}

void FinalDiscurso_Desenhar(const Assets *assets) {
    Fundo(momento == 0 ? assets->ato3Auditorio : assets->ato3FinalBastidores);
    if (momento == 0) {
        DrawRectangle(0,0,TELA_LARGURA,TELA_ALTURA,Fade((Color){24,16,30,255},0.47f));
        Texture2D lia = assets->lia07Envergonhada;
        if (!lia.id) lia=assets->lia07NervosaPalco;
        if (lia.id && lia.height>0) {
            float h=624, w=h*lia.width/lia.height;
            DrawTexturePro(lia,(Rectangle){0,0,lia.width,lia.height},
                (Rectangle){940-w/2,TELA_ALTURA-h,w,h},(Vector2){0,0},0,WHITE);
        }
    }
    DrawRectangleGradientH(0,0,TELA_LARGURA,TELA_ALTURA,
        Fade((Color){18,16,29,255},momento==2?0.97f:0.90f),
        Fade((Color){18,16,29,255},momento==2?0.36f:0.05f));
    UiDrawText("RE:7  /  ATO 3",80,55,18,ROSA);
    UiDrawText("F11  tela cheia",1090,55,16,Fade(MARFIM,0.7f));
    if (momento == 0) {
        Linha("QUANDO A VOZ FALHA",202,18,ROSA);
        UiDrawTextEmphasis("O silêncio pesa.",80,246,42,MARFIM);
        Linha("Lia tenta retomar a frase,",322,25,MARFIM);
        Linha("mas as palavras não vêm.",359,25,MARFIM);
        Linha("Ela abaixa o papel e deixa o palco",426,23,MARFIM);
        Linha("antes de terminar o discurso.",461,23,MARFIM);
    } else if (momento == 1) {
        Linha("DO OUTRO LADO DA CORTINA",187,18,ROSA);
        UiDrawTextEmphasis("Ficou tanto por dizer.",80,230,36,MARFIM);
        Linha("\"Eu sabia o que queria dizer...",310,25,MARFIM);
        Linha("Só não consegui colocar para fora.\"",347,25,MARFIM);
        Linha("O concurso continua no auditório.",420,22,MARFIM);
        Linha("Lia fica nos bastidores, com o discurso",455,22,MARFIM);
        Linha("ainda apertado entre as mãos.",490,22,MARFIM);
    } else {
        UiDrawTextEmphasis("Palavras em silêncio",80,210,37,MARFIM);
        Linha("Desta vez, Lia não terminou o discurso.",283,23,MARFIM);
        Linha("O papel guarda o que ela queria dizer.",318,23,MARFIM);
        const char *nomes[]={"Jogar novamente", "Voltar ao menu", "Sair do jogo"};
        const char *teclas[]={"ENTER", "M", "ESC"};
        for(int i=0;i<3;i++) {
            Rectangle r=BotaoFinal(i);
            bool hover=tempo>=BLOQUEIO && CheckCollisionPointRec(GetMousePosition(),r);
            DrawRectangleRounded(r,0.14f,8,hover?(Color){99,59,83,245}:(Color){40,34,51,225});
            DrawRectangleRoundedLines(r,0.14f,8,Fade(ROSA,hover?0.95f:0.35f));
            UiDrawText(nomes[i],(int)r.x+20,(int)r.y+14,22,MARFIM);
            UiDrawText(teclas[i],(int)(r.x+r.width-75),(int)r.y+18,16,ROSA);
        }
        Linha("Um novo jogo começa do início da história.",626,17,Fade(MARFIM,0.65f));
    }
    if (momento<2 && tempo>=BLOQUEIO) {
        Linha("CLIQUE, ESPAÇO ou ENTER para continuar",637,18,Fade(MARFIM,0.8f));
        for(int i=0;i<3;i++) DrawCircle(87+i*23,683,4,i==momento?ROSA:Fade(ROSA,0.3f));
    }
    // Entrada suave, sem trocar o volume escolhido pelo jogador.
    float alpha=1-fminf(tempo/0.40f,1);
    if(alpha>0) DrawRectangle(0,0,TELA_LARGURA,TELA_ALTURA,Fade(BLACK,alpha));
}
