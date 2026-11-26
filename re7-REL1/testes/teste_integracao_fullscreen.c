#include <stdio.h>
#include <assert.h>
#define SaveFileText Test_SaveFileText
#include "../src/core/game.c"
#include "../src/atos/ato3/entre_linhas.c"
bool Test_SaveFileText(const char *path, const char *text) { (void)path; (void)text; return true; }

static void RenderizarTeste(Assets *assets, RenderTexture2D quadro, const char *nome) {
    float escala = fminf((float)GetScreenWidth()/TELA_LARGURA, (float)GetScreenHeight()/TELA_ALTURA);
    float mx = (GetScreenWidth()-TELA_LARGURA*escala)/2;
    float my = (GetScreenHeight()-TELA_ALTURA*escala)/2;
    SetMouseOffset((int)-mx, (int)-my);
    SetMouseScale(1/escala, 1/escala);
    Rectangle botao = AreaBotao(1, Ato3_Etapa(desafio.modo, desafio.etapa)->total);
    SetMousePosition((int)(mx+(botao.x+botao.width/2)*escala), (int)(my+(botao.y+botao.height/2)*escala));
    assert(CheckCollisionPointRec(GetMousePosition(),botao));
    BeginTextureMode(quadro);
    DrawAto3EntreLinhas(assets);
    EndTextureMode();
    // Captura a composicao final em uma textura com o tamanho real da janela.
    RenderTexture2D saida = LoadRenderTexture(GetScreenWidth(),GetScreenHeight());
    BeginTextureMode(saida);
    ClearBackground(BLACK);
    DrawTexturePro(quadro.texture,(Rectangle){0,0,TELA_LARGURA,-TELA_ALTURA},
        (Rectangle){mx,my,TELA_LARGURA*escala,TELA_ALTURA*escala},(Vector2){0,0},0,WHITE);
    EndTextureMode();
    Image imagem=LoadImageFromTexture(saida.texture);
    ImageFlipVertical(&imagem);
    assert(ExportImage(imagem,nome));
    UnloadImage(imagem); UnloadRenderTexture(saida);
    printf("Render e mouse OK: %dx%d\n",GetScreenWidth(),GetScreenHeight());
}

int main(void) {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN | FLAG_WINDOW_RESIZABLE);
    InitWindow(TELA_LARGURA,TELA_ALTURA,"Teste de integracao RE7");
    SetExitKey(KEY_NULL);
    UiLoadFonts();
    Assets assets={0}; LoadAssets(&assets);
    assert(assets.ato3Auditorio.id && assets.ato3Pulpito.id && assets.ato3PcLigado.id);
    assert(assets.ato3PcDesligado.id && assets.ato3Papel.id && assets.ato3PlateiaAtenta.id);
    assert(assets.ato3PlateiaAplausos.id && assets.lia07NervosaPalco.id && assets.lia07ConfiantePalco.id);
    RenderTexture2D quadro=LoadRenderTexture(TELA_LARGURA,TELA_ALTURA);
    SetTextureFilter(quadro.texture,TEXTURE_FILTER_BILINEAR);
    Tokens_Restaurar(3,false);
    InitAto3EntreLinhas(ATO3_AJUDA_SOZINHA);
    RenderizarTeste(&assets,quadro,"validacao/janela.png");
    ToggleBorderlessWindowed();
    assert(IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE));
    RenderizarTeste(&assets,quadro,"validacao/fullscreen.png");
    Oratoria_Atualizar(&oratoria,0.3f);
    Oratoria_Selecionar(&oratoria,1);
    Oratoria_Digitar(&oratoria,'p');
    Oratoria_Digitar(&oratoria,'u');
    RenderizarTeste(&assets,quadro,"validacao/digitacao.png");
    ToggleBorderlessWindowed();
    assert(!IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE));
    SetWindowSize(1000,800);
    InitAto3EntreLinhas(ATO3_AJUDA_TIRAR_OPCAO);
    RenderizarTeste(&assets,quadro,"validacao/ajuda-4x3.png");
    for(int tokens=0;tokens<=3;tokens++) for(int erros=0;erros<=3;erros++) {
        Tokens_Restaurar(tokens,false);
        InitAto3EntreLinhas(ATO3_AJUDA_SOZINHA);
        for(int i=0;i<3;i++) {
            Oratoria_Atualizar(&oratoria,0.3f);
            const Ato3Etapa *e=Ato3_Etapa(desafio.modo,i);
            assert(Oratoria_Selecionar(&oratoria,i<erros?(e->correta+1)%e->total:e->correta));
            while(oratoria.digitando) Oratoria_Digitar(&oratoria,oratoria.letras[oratoria.progresso]);
            if(i<erros) {
                Oratoria_Atualizar(&oratoria,0.3f); assert(Oratoria_Avancar(&oratoria));
                Oratoria_Atualizar(&oratoria,0.3f); assert(Oratoria_Avancar(&oratoria));
            } else Oratoria_Atualizar(&oratoria,1.0f);
        }
        Oratoria_Atualizar(&oratoria,EL_TEMPO_CONCLUSAO);
        assert(Oratoria_Avancar(&oratoria));
        assert(Ato3EntreLinhas_FinalRuim()==(erros>=2));
        GameState game={0};
        ConcluirEntreLinhas(&game);
        if(erros>=2) assert(game.currentScreen==SCREEN_ENDING && game.motivoFinalRuim==FINAL_RUIM_DISCURSO);
        else {
            assert(game.currentScene==SCENE_ATO3_CONCLUSAO && game.currentScreen==SCREEN_GAME);
            FimDaCena(&game,GetScene(game.currentScene));
            if(tokens==0) assert(game.currentScreen==SCREEN_ENDING && game.motivoFinalRuim==FINAL_RUIM_SEM_TOKENS);
            else assert(game.currentScene==SCENE_ATO3_ETAPA_CONCLUIDA);
        }
        printf("Campanha: %d tokens / %d erros OK\n",tokens,erros);
    }
#ifdef RE7_DEV_MODE
    GameState preview={0};
    Tokens_Restaurar(3,false);
    Game_ForcarFinalRuim(&preview,FINAL_RUIM_SEM_TOKENS);
    assert(preview.currentScreen==SCREEN_ENDING && preview.motivoFinalRuim==FINAL_RUIM_SEM_TOKENS);
    assert(Tokens_Valor()==0);
    Game_ForcarFinalRuim(&preview,FINAL_RUIM_DISCURSO);
    assert(preview.currentScreen==SCREEN_ENDING && preview.motivoFinalRuim==FINAL_RUIM_DISCURSO);
    puts("Previa dos dois finais no modo desenvolvedor: OK");
#endif
    UnloadRenderTexture(quadro); UnloadAssets(&assets); UiUnloadFonts(); CloseWindow();
    puts("Integracao fullscreen, mouse, assets e finais: OK");
    return 0;
}
