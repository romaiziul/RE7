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

    static const char *lia07Paths[11] =
    {
        "assets/comum/lia07_neutra.png",
        "assets/comum/lia07_confusa.png",
        "assets/comum/lia07_brava.png",
        "assets/comum/lia07_surpresa.png",
        "assets/comum/lia07_contrariada.png",
        "assets/comum/lia07_nostalgica.png",
        "assets/comum/lia07_rindo.png",
        "assets/comum/lia07_feliz.png",
        "assets/comum/lia07_triste.png",
        "assets/comum/lia07_pensativa.png",
        "assets/comum/lia07_determinada.png"
    };

    for (int i = 0; i < 3; i++) assets->lia26[i] = LoadTexture(lia26Paths[i]);
    for (int i = 0; i < 11; i++) assets->lia07[i] = LoadTexture(lia07Paths[i]);

    assets->liaFrente = assets->lia26[0];
    assets->liaJovemLado = assets->lia07[0];

    assets->amigo[0] = LoadTexture("assets/comum/amigo_neutro.png");
    assets->amigo[1] = LoadTexture("assets/comum/amigo_feliz.png");

    assets->amigoSurpreso[0] = LoadTexture("assets/comum/amigo_surpreso_esquerda.png");
    assets->amigoSurpreso[1] = LoadTexture("assets/comum/amigo_surpreso_direita.png");


    // ATO 1 E ATO 2

    assets->cgTela2026 = LoadTexture("assets/ato1/ato1_cg_tela2026.png");

    assets->bgCorredor = LoadTexture("assets/ato2/ato2_corredor.png");
    assets->bgLanhouse = LoadTexture("assets/ato2/ato2_lanhouse.png");
    assets->poster = LoadTexture("assets/ato2/ato2_poster.png");
    assets->cgEscrevem = LoadTexture("assets/ato2/ato2_cg_escrevem.png");
    assets->cgProfessora = LoadTexture("assets/ato2/ato2_cg_professora.png");
    assets->cgEnsaio = LoadTexture("assets/ato2/ato2_cg_ensaio.png");
    assets->cgCasaNoite = LoadTexture("assets/ato2/ato2_cg_casa_noite.png");
    assets->cgFrenteEscola = LoadTexture("assets/ato2/ato2_cg_frente_escola.png");
    assets->pcBoot = LoadTexture("assets/ato2/ato2_doors_boot.png");
    assets->pcDesktop = LoadTexture("assets/ato2/ato2_doors_xp.png");
    assets->pcPerfil = LoadTexture("assets/ato2/ato2_cg_perfil.png");


    // ATO 3

    assets->bgAuditorio = LoadTexture("assets/ato3/ato3_auditorio.png");
    assets->monitorCrt = LoadTexture("assets/ato3/ato3_monitor_crt.png");
    assets->papelDiscurso = LoadTexture("assets/ato3/ato3_papel_discurso.png");

    SetTextureFilter(assets->bgAuditorio, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(assets->monitorCrt, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(assets->papelDiscurso, TEXTURE_FILTER_BILINEAR);
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
    for (int i = 0; i < 11; i++) UnloadTexture(assets->lia07[i]);
    for (int i = 0; i < 2; i++) UnloadTexture(assets->amigo[i]);
    for (int i = 0; i < 2; i++) UnloadTexture(assets->amigoSurpreso[i]);

    UnloadTexture(assets->cgTela2026);
    UnloadTexture(assets->bgCorredor);
    UnloadTexture(assets->bgLanhouse);
    UnloadTexture(assets->poster);
    UnloadTexture(assets->cgEscrevem);
    UnloadTexture(assets->cgProfessora);
    UnloadTexture(assets->cgEnsaio);
    UnloadTexture(assets->cgCasaNoite);
    UnloadTexture(assets->cgFrenteEscola);
    UnloadTexture(assets->pcBoot);
    UnloadTexture(assets->pcDesktop);
    UnloadTexture(assets->pcPerfil);
    UnloadTexture(assets->bgAuditorio);
    UnloadTexture(assets->monitorCrt);
    UnloadTexture(assets->papelDiscurso);
    UnloadTexture(assets->logo);
    UnloadTexture(assets->iaStar);
}