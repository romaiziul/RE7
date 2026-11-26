#include "assets.h"


// Mipmaps mantem os recortes suaves quando reduzidos para o palco.
static Texture2D CarregarArteAto3(const char *path)
{
    Texture2D t = LoadTexture(path);
    if (t.id) {
        GenTextureMipmaps(&t);
        SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
    }
    return t;
}

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
    assets->ato3FinalSemRetorno = CarregarArteAto3("assets/ato3/ato3_final_sem_retorno.png");
    assets->lia07SemRetorno = CarregarArteAto3("assets/comum/lia07_sem_retorno.png");
    assets->ato3FinalBastidores = CarregarArteAto3("assets/ato3/ato3_final_bastidores.png");
    assets->lia07Envergonhada = CarregarArteAto3("assets/comum/lia07_envergonhada.png");
    assets->ato3Auditorio = CarregarArteAto3("assets/ato3/ato3_auditorio.png");
    assets->ato3Pulpito = CarregarArteAto3("assets/ato3/ato3_pulpito.png");
    assets->ato3PcDesligado = CarregarArteAto3("assets/ato3/ato3_pc_desligado.png");
    assets->ato3PcLigado = CarregarArteAto3("assets/ato3/ato3_pc_ligado.png");
    assets->ato3Papel = CarregarArteAto3("assets/ato3/ato3_papel_discurso.png");
    assets->ato3PlateiaAtenta = CarregarArteAto3("assets/ato3/ato3_plateia_atenta.png");
    assets->ato3PlateiaAplausos = CarregarArteAto3("assets/ato3/ato3_plateia_aplausos.png");
    assets->lia07NervosaPalco = CarregarArteAto3("assets/comum/lia07_nervosa_palco.png");
    assets->lia07ConfiantePalco = CarregarArteAto3("assets/comum/lia07_confiante_palco.png");
}


void UnloadAssets(Assets *assets)
{
    if (assets->ato3FinalSemRetorno.id) UnloadTexture(assets->ato3FinalSemRetorno);
    assets->ato3FinalSemRetorno = (Texture2D){0};
    if (assets->lia07SemRetorno.id) UnloadTexture(assets->lia07SemRetorno);
    assets->lia07SemRetorno = (Texture2D){0};
    if (assets->ato3FinalBastidores.id) UnloadTexture(assets->ato3FinalBastidores);
    assets->ato3FinalBastidores = (Texture2D){0};
    if (assets->lia07Envergonhada.id) UnloadTexture(assets->lia07Envergonhada);
    assets->lia07Envergonhada = (Texture2D){0};
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
    UnloadTexture(assets->logo);
    UnloadTexture(assets->iaStar);
    if (assets->ato3Auditorio.id) UnloadTexture(assets->ato3Auditorio);
    assets->ato3Auditorio = (Texture2D){0};
    if (assets->ato3Pulpito.id) UnloadTexture(assets->ato3Pulpito);
    assets->ato3Pulpito = (Texture2D){0};
    if (assets->ato3PcDesligado.id) UnloadTexture(assets->ato3PcDesligado);
    assets->ato3PcDesligado = (Texture2D){0};
    if (assets->ato3PcLigado.id) UnloadTexture(assets->ato3PcLigado);
    assets->ato3PcLigado = (Texture2D){0};
    if (assets->ato3Papel.id) UnloadTexture(assets->ato3Papel);
    assets->ato3Papel = (Texture2D){0};
    if (assets->ato3PlateiaAtenta.id) UnloadTexture(assets->ato3PlateiaAtenta);
    assets->ato3PlateiaAtenta = (Texture2D){0};
    if (assets->ato3PlateiaAplausos.id) UnloadTexture(assets->ato3PlateiaAplausos);
    assets->ato3PlateiaAplausos = (Texture2D){0};
    if (assets->lia07NervosaPalco.id) UnloadTexture(assets->lia07NervosaPalco);
    assets->lia07NervosaPalco = (Texture2D){0};
    if (assets->lia07ConfiantePalco.id) UnloadTexture(assets->lia07ConfiantePalco);
    assets->lia07ConfiantePalco = (Texture2D){0};
}
