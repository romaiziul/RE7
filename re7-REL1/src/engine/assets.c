#include "assets.h"


void LoadAssets(Assets *assets)
{

    // BACKGROUNDS

    assets->bgPresent = LoadTexture("assets/ato1/ato1_quarto2026.png");

    assets->bg2007 = LoadTexture("assets/ato1/ato1_quarto2007.png");

    assets->bgRua = LoadTexture("assets/comum/rua.png");


    // BOTOES DO MENU (normal, hover, pressed)

    static const char *menuPaths[4][3] =
    {
        {"assets/comum/novoJogo.png",  "assets/comum/novoJogoHover.png",  "assets/comum/novoJogoPressed.png"},
        {"assets/comum/continuar.png", "assets/comum/continuarHover.png", "assets/comum/continuarPressed.png"},
        {"assets/comum/creditos.png",  "assets/comum/creditosHover.png",  "assets/comum/creditosPressed.png"},
        {"assets/comum/sair.png",      "assets/comum/sairHover.png",      "assets/comum/sairPressed.png"}
    };

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            assets->menuButtons[i][j] = LoadTexture(menuPaths[i][j]);


    // LOGO

    assets->logo = LoadTexture("assets/comum/logo.png");

    // AVATAR DA IA

    assets->iaStar = LoadTexture("assets/comum/iaStar.png");


    // PERSONAGENS

    static const char *lia26Paths[3] =
    {
        "assets/comum/lia26_neutra.png",
        "assets/comum/lia26_preocupada.png",
        "assets/comum/lia26_decidida.png"
    };

    static const char *lia07Paths[7] =
    {
        "assets/comum/lia07_neutra.png",
        "assets/comum/lia07_confusa.png",
        "assets/comum/lia07_brava.png",
        "assets/comum/lia07_surpresa.png",
        "assets/comum/lia07_contrariada.png",
        "assets/comum/lia07_nostalgica.png",
        "assets/comum/lia07_rindo.png"
    };

    for (int i = 0; i < 3; i++) assets->lia26[i] = LoadTexture(lia26Paths[i]);
    for (int i = 0; i < 7; i++) assets->lia07[i] = LoadTexture(lia07Paths[i]);

    assets->liaFrente = assets->lia26[0];
    assets->liaJovemLado = assets->lia07[0];

    assets->amigo[0] = LoadTexture("assets/comum/amigo_neutro.png");
    assets->amigo[1] = LoadTexture("assets/comum/amigo_feliz.png");


    // ATO 1 E ATO 2

    assets->cgTela2026 = LoadTexture("assets/ato1/ato1_cg_tela2026.png");

    assets->bgCorredor = LoadTexture("assets/ato2/ato2_corredor.png");
    assets->poster = LoadTexture("assets/ato2/ato2_poster.png");
    assets->cgEscrevem = LoadTexture("assets/ato2/ato2_cg_escrevem.png");
    assets->cgProfessora = LoadTexture("assets/ato2/ato2_cg_professora.png");
    assets->cgEnsaio = LoadTexture("assets/ato2/ato2_cg_ensaio.png");
}


void UnloadAssets(Assets *assets)
{
    UnloadTexture(assets->bgPresent);
    UnloadTexture(assets->bg2007);
    UnloadTexture(assets->bgRua);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            UnloadTexture(assets->menuButtons[i][j]);

    for (int i = 0; i < 3; i++) UnloadTexture(assets->lia26[i]);
    for (int i = 0; i < 7; i++) UnloadTexture(assets->lia07[i]);
    for (int i = 0; i < 2; i++) UnloadTexture(assets->amigo[i]);

    UnloadTexture(assets->cgTela2026);
    UnloadTexture(assets->bgCorredor);
    UnloadTexture(assets->poster);
    UnloadTexture(assets->cgEscrevem);
    UnloadTexture(assets->cgProfessora);
    UnloadTexture(assets->cgEnsaio);
    UnloadTexture(assets->logo);
    UnloadTexture(assets->iaStar);
}