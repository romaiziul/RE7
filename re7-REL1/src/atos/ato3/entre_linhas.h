// entre_linhas.h
// Desafio 3 (Ato 3): "ENTRE AS LINHAS". Recuperar as tres palavras borradas
// do papel do discurso. As regras ficam em ato3_logica.c; aqui so a tela.
#ifndef ENTRE_LINHAS_H
#define ENTRE_LINHAS_H

#include <stdbool.h>

#include "assets.h"
#include "minigame.h"
#include "ato3_logica.h"

// Comeca o desafio no modo escolhido (tirar uma opcao errada ou sozinha).
void InitAto3EntreLinhas(Ato3Ajuda modo);

// Uma vez por quadro. MINIGAME_VENCEU quando as tres palavras foram
// recuperadas e a jogadora continuou depois da conclusao.
MinigameResult UpdateAto3EntreLinhas(void);

void DrawAto3EntreLinhas(Assets *assets);

// Modo desenvolvedor: pula a fala/animacao atual (false se ha uma escolha
// na tela) e resolve a etapa atual com a palavra certa.
bool Ato3EntreLinhas_DevPular(void);
void Ato3EntreLinhas_DevResolverEtapa(void);

#endif
