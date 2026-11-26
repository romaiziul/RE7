// Ato 3: escolha direcional + digitacao, mantendo a API usada pelo game.c.
#include "entre_linhas.h"
#include "oratoria_typing.h"
#include "raylib.h"
#include "ui.h"
#include "ato3.h"
#include "tokens.h"
#include "audio_manager.h"
#include <math.h>
#include <stddef.h>

static Oratoria oratoria;
// Mantem compatibilidade com o gerador de capturas antigo.
#define desafio (oratoria.discurso)
static float tremorErro, animacaoLetra, avisoErro;
static int progressoAnterior;
static bool aguardandoSoltar;
static int teclaSelecao = KEY_NULL;

bool Ato3EntreLinhas_FinalRuim(void) {
    int erros = 0;
    for (int i = 0; i < ATO3_ETAPAS; i++)
        if (desafio.errou[i]) erros++;
    // So decide o desfecho depois das tres rodadas.
    return desafio.fase >= EL_FASE_CONCLUSAO && erros >= 2;
}

#define COR_ROSA (Color){227,93,215,255}
#define COR_AZUL_MSN (Color){39,172,234,255}
#define COR_MARINHO (Color){29,38,96,255}
#define COR_VERDE (Color){70,172,127,255}
#define COR_CLARA (Color){232,240,250,255}
#define COR_ERRO (Color){202,50,87,255}
#define TREMOR_ERRO_SEGUNDOS 0.3f

static const Rectangle PAPEL = {220,100,840,274};
static const Rectangle JANELA = {230,394,820,260};

static void LimparCaracteres(void) { while (GetCharPressed() != 0) {} }
static float Limitar01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }
static void Centralizado(const char *s, int y, int fs, Color cor) {
    UiDrawText(s, (1280-UiMeasureText(s,fs))/2, y, fs, cor);
}

void InitAto3EntreLinhas(Ato3Ajuda modo) {
    Oratoria_Iniciar(&oratoria, modo, (unsigned)GetRandomValue(1, 2147483646));
    tremorErro = animacaoLetra = avisoErro = 0;
    progressoAnterior = 0;
    aguardandoSoltar = false;
    teclaSelecao = KEY_NULL;
    LimparCaracteres();
}

static Rectangle AreaBotao(int indice, int total) {
    // Dois botoes centralizados; tres mantem as posicoes originais.
    float inicio = total == 2 ? JANELA.x + (JANELA.width - 500)/2 : JANELA.x + 22;
    return (Rectangle){inicio + indice*260, JANELA.y+66, 240, 78};
}

static void FeedbackErro(void) {
    AudioManager_Play(SOM_ERRO);
    tremorErro = TREMOR_ERRO_SEGUNDOS;
    avisoErro = 0.65f;
}

MinigameResult UpdateAto3EntreLinhas(void) {
    float dt = GetFrameTime();
    if (!IsWindowFocused()) { LimparCaracteres(); return MINIGAME_CONTINUA; }
    if (tremorErro > 0) tremorErro -= dt;
    if (animacaoLetra > 0) animacaoLetra -= dt;
    if (avisoErro > 0) avisoErro -= dt;

    if (aguardandoSoltar) {
        // A tecla que escolheu nunca vira uma letra da resposta, nem por repeticao.
        aguardandoSoltar = IsKeyDown(teclaSelecao);
        if (aguardandoSoltar) {
            LimparCaracteres();
            return MINIGAME_CONTINUA;
        }
    }

    bool digitava = oratoria.digitando;
    Oratoria_Atualizar(&oratoria, dt);
    if (digitava && !oratoria.digitando && oratoria.esgotou) FeedbackErro();

    if (oratoria.digitando) {
        int c;
        while ((c = GetCharPressed()) != 0) {
            int antes = oratoria.progresso;
            OratoriaEntrada resposta = Oratoria_Digitar(&oratoria, c);
            if (resposta == ORATORIA_LETRA_ERRADA || resposta == ORATORIA_PALAVRA_ERRADA)
                FeedbackErro();
            if (oratoria.progresso > antes) {
                progressoAnterior = antes;
                animacaoLetra = 0.17f;
                AudioManager_Play(resposta == ORATORIA_PALAVRA_CERTA ? SOM_ACERTO : SOM_DIALOGO);
            }
        }
    } else if (desafio.fase == EL_FASE_ESCOLHENDO) {
        LimparCaracteres();
        const Ato3Etapa *etapa = Ato3_Etapa(desafio.modo, desafio.etapa);
        int selecionada = -1;
        int tecla = KEY_NULL;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            selecionada = 0;
            tecla = IsKeyPressed(KEY_LEFT) ? KEY_LEFT : KEY_A;
        } else if (etapa->total == 3 && (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))) {
            selecionada = 1;
            tecla = IsKeyPressed(KEY_UP) ? KEY_UP : KEY_W;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            selecionada = etapa->total-1;
            tecla = IsKeyPressed(KEY_RIGHT) ? KEY_RIGHT : KEY_D;
        }
        for (int i=0;i<etapa->total;i++) {
            if (IsButtonClicked(AreaBotao(i, etapa->total))) { selecionada = i; tecla = KEY_NULL; }
        }
        if (Oratoria_Selecionar(&oratoria, selecionada)) {
            AudioManager_Play(SOM_MOUSE);
            teclaSelecao = tecla;
            aguardandoSoltar = tecla != KEY_NULL;
            animacaoLetra = avisoErro = 0;
        }
    } else {
        LimparCaracteres();
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            if (Oratoria_Avancar(&oratoria)) AudioManager_Play(SOM_DIALOGO);
    }
    return EntreLinhas_Terminou(&desafio) ? MINIGAME_VENCEU : MINIGAME_CONTINUA;
}

static void DesenharRetrato(Texture2D tex, float altura, float x, float y) {
    if (!tex.id || tex.height <= 0) return;
    float largura = altura * (float)tex.width/tex.height;
    DrawTexturePro(tex,(Rectangle){0,0,(float)tex.width,(float)tex.height},
        (Rectangle){x,y,largura,altura},(Vector2){0,0},0,WHITE);
}

static void DesenharSeta(int coluna, float x, float y, Color cor) {
    Vector2 ponta = {x+(coluna==0?-8:coluna==2?8:0), y+(coluna==1?-8:0)};
    Vector2 cauda = {x+(coluna==0?8:coluna==2?-8:0), y+(coluna==1?8:0)};
    DrawLineEx(cauda,ponta,2.5f,cor);
    if (coluna==1) {
        DrawLineEx(ponta,(Vector2){x-6,y-2},2.5f,cor);
        DrawLineEx(ponta,(Vector2){x+6,y-2},2.5f,cor);
    } else {
        float dx = coluna==0?6:-6;
        DrawLineEx(ponta,(Vector2){ponta.x+dx,y-6},2.5f,cor);
        DrawLineEx(ponta,(Vector2){ponta.x+dx,y+6},2.5f,cor);
    }
}

static void Moldura(const char *titulo) {
    DrawRectangleRounded((Rectangle){JANELA.x+5,JANELA.y+7,JANELA.width,JANELA.height},0.05f,8,Fade(BLACK,0.35f));
    DrawRectangleRounded(JANELA,0.05f,8,COR_CLARA);
    DrawRectangleRounded((Rectangle){JANELA.x,JANELA.y,JANELA.width,42},0.3f,8,COR_AZUL_MSN);
    DrawRectangleRoundedLines(JANELA,0.05f,8,Fade(WHITE,0.8f));
    DrawCircle((int)JANELA.x+24,(int)JANELA.y+21,6,WHITE);
    UiDrawTextEmphasis(titulo,(int)JANELA.x+42,(int)JANELA.y+10,21,WHITE);
    UiDrawText("_  x",(int)(JANELA.x+JANELA.width-58),(int)JANELA.y+10,20,WHITE);
}

// Oculta somente a exibicao da alternativa, sem alterar a resposta original.
// Conta caracteres UTF-8 para preservar letras acentuadas.
static void OcultarMeioPalavra(const char *original, char *saida, int capacidade) {
    int total = GetCodepointCount(original);
    int pos = 0;
    for (int i = 0; *original && pos < capacidade-1; i++) {
        int bytes = 0;
        GetCodepointNext(original, &bytes);
        if (bytes <= 0) break;
        if (i > 0 && i < total-2) {
            saida[pos++] = '.';
        } else {
            if (pos + bytes >= capacidade) break;
            for (int j = 0; j < bytes; j++) saida[pos++] = original[j];
        }
        original += bytes;
    }
    saida[pos] = '\0';
    if (pos > 0) {
        int bytes = 0;
        int letra = GetCodepointNext(saida, &bytes);
        if ((letra >= 'A' && letra <= 'Z') ||
            (letra >= 0xC0 && letra <= 0xDE && letra != 0xD7)) {
            int n = 0;
            const char *minuscula = CodepointToUTF8(letra + 32, &n);
            if (n == bytes)
                for (int i = 0; i < n; i++) saida[i] = minuscula[i];
        }
    }
}

static void DesenharJanelaOpcoes(void) {
    const Ato3Etapa *etapa = Ato3_Etapa(desafio.modo, desafio.etapa);
    Moldura("Re:7 Messenger  /  escolha sua palavra");
    const char *teclas[3] = {"A","W","D"};
    for (int indice=0;indice<etapa->total;indice++) {
        int coluna = etapa->total == 2 ? indice*2 : indice;
        Rectangle r=AreaBotao(indice, etapa->total);
        bool hover = CheckCollisionPointRec(GetMousePosition(),r);
        DrawRectangleRounded(r,0.16f,8,hover?COR_ROSA:COR_MARINHO);
        char palavra[128];
        OcultarMeioPalavra(etapa->opcoes[indice], palavra, sizeof(palavra));
        int fs = 25;
        while (fs>18 && UiMeasureText(palavra,fs)>r.width-20) fs--;
        UiDrawTextEmphasis(palavra,(int)(r.x+(r.width-UiMeasureTextEmphasis(palavra,fs))/2),(int)r.y+14,fs,WHITE);
        DesenharSeta(coluna,r.x+r.width/2-22,r.y+58,WHITE);
        UiDrawText(teclas[coluna],(int)(r.x+r.width/2+4),(int)r.y+47,19,WHITE);
    }
    Centralizado("Escolha pelo sentido da frase. Depois, digite a palavra.",555,20,COR_MARINHO);
    Centralizado(etapa->total==2?"Escolha com A / seta esquerda ou D / seta direita.":"Você está por conta própria. Encontre a sua voz.",589,18,(Color){81,96,127,255});
    Centralizado("O tempo começa na digitação.  |  ESC: pausar",622,16,(Color){81,96,127,255});
}

// Posicao de uma letra: prefixo resolvido + restantes na ordem embaralhada.
static int PosicaoLetra(int identidade, int progresso) {
    if (identidade<progresso) return identidade;
    int pos=progresso;
    for (int i=0;i<oratoria.tamanho;i++) {
        int id=oratoria.ordem[i];
        if (id<progresso) continue;
        if (id==identidade) return pos;
        pos++;
    }
    return identidade;
}

static void DesenharDigitacao(void) {
    Moldura("Discurso.txt  /  organize sua voz");
    bool acertou = desafio.fase==EL_FASE_ACERTO;
    Centralizado(acertou?"Palavra recuperada!":(aguardandoSoltar?"Solte a tecla para começar":"Digite a palavra na ordem correta"),447,23,COR_MARINHO);
    const float passo=58, largura=50, y=489;
    float x0=640-(oratoria.tamanho*passo-8)/2;
    float t=1-Limitar01(animacaoLetra/0.17f);
    t=t*t*(3-2*t);
    for (int id=0;id<oratoria.tamanho;id++) {
        float pos=(float)PosicaoLetra(id,oratoria.progresso);
        if (animacaoLetra>0) {
            float anterior=(float)PosicaoLetra(id,progressoAnterior);
            pos=anterior+(pos-anterior)*t;
        }
        bool pronta=id<oratoria.progresso;
        float salto=pronta && id==oratoria.progresso-1 && animacaoLetra>0 ? sinf(t*PI)*8 : 0;
        Rectangle tile={x0+pos*passo,y-salto,largura,61};
        Color cor=pronta?COR_VERDE:COR_MARINHO;
        if (avisoErro>0 && !pronta) cor=(Color){130,52,93,255};
        DrawRectangleRounded((Rectangle){tile.x,tile.y+3,tile.width,tile.height},0.28f,12,Fade(COR_MARINHO,0.16f));
        DrawRectangleRounded(tile,0.28f,12,cor);
        DrawRectangleRoundedLines(tile,0.28f,12,Fade(WHITE,pronta?0.55f:0.20f));
        int bytes=0;
        const char *utf8=CodepointToUTF8(oratoria.letras[id],&bytes);
        char letra[8]={0};
        for (int k=0;k<bytes && k<7;k++) letra[k]=utf8[k];
        UiDrawTextEmphasis(letra,(int)(tile.x+(tile.width-UiMeasureTextEmphasis(letra,38))/2),(int)tile.y+7,38,WHITE);
        if (pronta) DrawRectangle((int)tile.x+13,(int)tile.y+51,24,3,Fade(WHITE,0.8f));
    }
    float proporcao=Limitar01(oratoria.restante/oratoria.limite);
    Color timer=proporcao<0.3f?COR_ERRO:COR_AZUL_MSN;
    DrawRectangleRounded((Rectangle){255,579,770,10},1,8,(Color){201,215,234,255});
    if (proporcao>0) DrawRectangleRounded((Rectangle){255,579,770*proporcao,10},1,8,timer);
    UiDrawText(TextFormat("%d / %d letras",oratoria.progresso,oratoria.tamanho),255,598,20,COR_MARINHO);
    const char *tempo=TextFormat("%.1f s",oratoria.restante);
    UiDrawTextEmphasis(tempo,1025-UiMeasureTextEmphasis(tempo,23),597,23,timer);
    Centralizado(avisoErro>0?"Letra incorreta: -0,35 s. Continue de onde parou.":"Acentos são opcionais. Letras certas ficam no lugar.",629,16,avisoErro>0?COR_ERRO:(Color){81,96,127,255});
}

void DrawAto3EntreLinhas(Assets *assets) {
    int recuperadas=EntreLinhas_Recuperadas(&desafio);
    bool concluiu=desafio.fase>=EL_FASE_CONCLUSAO;
    bool finalRuim=Ato3EntreLinhas_FinalRuim();
    Ato3Cena cena={.pcOnline=!Tokens_IAOffline(),.aplausos=concluiu&&!finalRuim,.professoraSorri=concluiu&&!finalRuim,
        .nervosismo=(float)(3-recuperadas)/3};
    Ato3_DesenharCenario(&cena,assets);
    DrawRectangle(0,0,1280,720,Fade(BLACK,concluiu?0.35f:0.56f));
    UiDrawTextEmphasis("ENTRE AS LINHAS",230,25,32,WHITE);
    UiDrawText(concluiu?(finalRuim?"O nervosismo toma conta.":"Você encontrou sua voz."):"Respire. Escolha. Digite.",232,66,20,(Color){180,217,246,255});
    for(int i=0;i<3;i++) {
        Color c=desafio.nitida[i]?(desafio.errou[i]?COR_AZUL_MSN:COR_VERDE):Fade(WHITE,0.25f);
        DrawCircle(817+i*36,54,11,c);
        UiDrawText(TextFormat("%d",i+1),812+i*36,42,18,WHITE);
    }
    Texture2D lia=(desafio.fase==EL_FASE_ACERTO||concluiu)&&!finalRuim?assets->lia07ConfiantePalco:assets->lia07NervosaPalco;
    DesenharRetrato(lia,410,1018,310);
    if (desafio.fase==EL_FASE_ERRO_AMIGO) DesenharRetrato(assets->amigo[0],410,-20,310);
    float nitidez[3];
    for(int i=0;i<3;i++) {
        nitidez[i]=desafio.nitida[i]?1:0;
        if (i==desafio.etapa && (desafio.fase==EL_FASE_ACERTO || desafio.fase==EL_FASE_ERRO_AMIGO))
            nitidez[i]=Limitar01(desafio.tempoFase/0.5f);
    }
    // Sem tremor continuo durante a digitacao: as letras precisam permanecer legiveis.
    Vector2 tremor={0,0};
    if(tremorErro>0) tremor.x=sinf((float)GetTime()*50)*4*tremorErro/TREMOR_ERRO_SEGUNDOS;
    Ato3_DesenharPapel(assets,PAPEL,29,nitidez,desafio.etapa<3?desafio.etapa:-1,tremor);
    const Ato3Etapa *etapa=Ato3_Etapa(desafio.modo,desafio.etapa);
    if(oratoria.digitando || desafio.fase==EL_FASE_ACERTO) DesenharDigitacao();
    else switch(desafio.fase) {
        case EL_FASE_ESCOLHENDO: DesenharJanelaOpcoes(); break;
        case EL_FASE_ERRO_LIA:
            DrawDialogueBox("Lia",oratoria.esgotou?"(gaguejando) Eu... perdi a palavra. Calma, respira...":etapa->falaErroLia);
            break;
        case EL_FASE_ERRO_AMIGO: DrawDialogueBox("João",etapa->falaAmigo); break;
        case EL_FASE_CONCLUSAO:
        case EL_FASE_FIM:
            DrawRectangleRounded((Rectangle){330,424,620,150},0.12f,8,COR_MARINHO);
            Centralizado(finalRuim?"Lia fica muito envergonhada.":"Discurso recuperado!",447,28,WHITE);
            Centralizado(finalRuim?"Ela se sente péssima e não consegue continuar.":"A voz continua sendo sua.",493,21,(Color){180,217,246,255});
            if(desafio.tempoFase>=EL_TEMPO_CONCLUSAO) Centralizado("ESPAÇO ou ENTER para continuar",540,18,WHITE);
            break;
        default: break;
    }
    if(!IsWindowFocused()) {
        DrawRectangle(0,0,1280,720,Fade(BLACK,0.65f));
        Centralizado("Pausado — volte à janela para continuar",340,28,WHITE);
    }
}

bool Ato3EntreLinhas_DevPular(void) {
    if(oratoria.digitando) return false;
    if(desafio.fase==EL_FASE_ACERTO) { Oratoria_Atualizar(&oratoria,EL_TEMPO_ACERTO); return true; }
    if(desafio.fase==EL_FASE_ESCOLHENDO) return false;
    desafio.bloqueio=0;
    if(desafio.fase==EL_FASE_CONCLUSAO) desafio.tempoFase=EL_TEMPO_CONCLUSAO;
    return Oratoria_Avancar(&oratoria);
}

void Ato3EntreLinhas_DevResolverEtapa(void) {
    if(desafio.fase==EL_FASE_ESCOLHENDO) {
        desafio.bloqueio=0;
        oratoria.digitando=false;
        Oratoria_Selecionar(&oratoria,Ato3_Etapa(desafio.modo,desafio.etapa)->correta);
        aguardandoSoltar=false;
        while(oratoria.digitando) Oratoria_Digitar(&oratoria,oratoria.letras[oratoria.progresso]);
        Oratoria_Atualizar(&oratoria,EL_TEMPO_ACERTO);
        return;
    }
    Ato3EntreLinhas_DevPular();
}
