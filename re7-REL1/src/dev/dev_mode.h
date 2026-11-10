// dev_mode.h
// Modo desenvolvedor: painel (F10) para navegar entre os atos e testar
// cenarios sem jogar a historia toda.
//
// So existe no build de desenvolvimento:  .\build.ps1 -Dev   (gera re7_dev.exe)
// No build normal (.\build.ps1, gera re7.exe) RE7_DEV_MODE nao e definido:
// o painel nao e compilado e as funcoes abaixo nao fazem nada.
#ifndef DEV_MODE_H
#define DEV_MODE_H

#include <stdbool.h>

#include "game.h"

#ifdef RE7_DEV_MODE

// Antes do UpdateGame: atalhos e cliques no painel.
void DevMode_Atualizar(GameState *game);

// true se o painel usou o clique deste quadro (o jogo nao deve recebe-lo).
bool DevMode_CapturouEntrada(void);

// Depois do DrawGame: painel e indicador "DEV".
void DevMode_Desenhar(const GameState *game);

#else

static inline void DevMode_Atualizar(GameState *game) { (void)game; }
static inline bool DevMode_CapturouEntrada(void) { return false; }
static inline void DevMode_Desenhar(const GameState *game) { (void)game; }

#endif

#endif
