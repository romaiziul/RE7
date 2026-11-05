#ifndef STORY_ATO1_H
#define STORY_ATO1_H

#include "scenes.h"

// Cena 1: quarto em 2026
extern Dialogue ato1PresenteDialogues[];
int GetAto1PresenteCount(void);

// Cena 2: acorda em 2007 (antes de explorar o quarto)
extern Dialogue ato1Quarto2007Dialogues[];
int GetAto1Quarto2007Count(void);

// Cena 2: o PC esta bloqueado (antes do minijogo da senha)
extern Dialogue ato1PcDialogues[];
int GetAto1PcCount(void);

// Cena 3: a IA explica (antes da escolha do ano)
extern Dialogue ato1IaADialogues[];
int GetAto1IaACount(void);

// Cena 3: depois do ano certo
extern Dialogue ato1IaBDialogues[];
int GetAto1IaBCount(void);

// Escolha 2: em que ano voce esta?
#define ATO1_ESCOLHA_ANO_PERGUNTA "Em que ano você está?"
#define ATO1_ESCOLHA_ANO_A "2005"
#define ATO1_ESCOLHA_ANO_B "2006"
#define ATO1_ESCOLHA_ANO_C "2007"

#endif
