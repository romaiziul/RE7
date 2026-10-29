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


    // PERSONAGEM

    assets->liaFrente = LoadTexture("assets/comum/lia26_neutra.png");

    assets->liaJovemLado = LoadTexture("assets/comum/lia07_neutra.png");
}


void UnloadAssets(Assets *assets)
{
    UnloadTexture(assets->bgPresent);
    UnloadTexture(assets->bg2007);
    UnloadTexture(assets->bgRua);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            UnloadTexture(assets->menuButtons[i][j]);

    UnloadTexture(assets->liaFrente);
    UnloadTexture(assets->liaJovemLado);
    UnloadTexture(assets->logo);
    UnloadTexture(assets->iaStar);
}