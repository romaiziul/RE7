// Compilar somente este .c: ele inclui assets.c para observar o ciclo de vida.
#include "raylib.h"
#include "assets.h"
#include <stdio.h>
#include <string.h>

static const char *paths[] = {
    "assets/ato3/ato3_auditorio.png", "assets/ato3/ato3_pulpito.png",
    "assets/ato3/ato3_pc_desligado.png", "assets/ato3/ato3_pc_ligado.png",
    "assets/ato3/ato3_papel_discurso.png", "assets/ato3/ato3_plateia_atenta.png",
    "assets/ato3/ato3_plateia_aplausos.png",
    "assets/comum/lia07_nervosa_palco.png", "assets/comum/lia07_confiante_palco.png"
};
static unsigned int ids[9];
static int loads[9], unloads[9];

static Texture2D CarregarObservado(const char *path)
{
    Texture2D t = LoadTexture(path);
    for (int i = 0; i < 9; i++) if (strcmp(path, paths[i]) == 0) {
        loads[i]++;
        if (t.width > 0 && t.height > 0) ids[i] = t.id;
    }
    return t;
}
static void DescarregarObservado(Texture2D t)
{
    for (int i = 0; i < 9; i++) if (ids[i] && t.id == ids[i]) unloads[i]++;
    UnloadTexture(t);
}
#define LoadTexture CarregarObservado
#define UnloadTexture DescarregarObservado
#include "../src/engine/assets.c"
#undef LoadTexture
#undef UnloadTexture

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(1280, 720, "Teste assets Ato 3");
    int failures = 0;
    for (int cycle = 0; cycle < 2; cycle++) {
        memset(ids, 0, sizeof(ids));
        memset(loads, 0, sizeof(loads));
        memset(unloads, 0, sizeof(unloads));
        Assets assets = {0};
        LoadAssets(&assets);
        UnloadAssets(&assets);
        for (int i = 0; i < 9; i++) {
            if (!ids[i] || loads[i] != 1 || unloads[i] != 1) {
                printf("FALHA ciclo %d: %s (id=%u, carga=%d, descarga=%d)\n",
                       cycle, paths[i], ids[i], loads[i], unloads[i]);
                failures++;
            }
        }
    }
    CloseWindow();
    printf("18 verificacoes de ciclo de vida, %d falhas\n", failures);
    return failures ? 1 : 0;
}
