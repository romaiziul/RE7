#ifndef STORY_ATO3_H
#define STORY_ATO3_H

#include "scenes.h"

// Falas do Ato 3 (docs/roteiro/ATO 3.docx).

extern Dialogue ato3AberturaDialogues[];
extern Dialogue ato3ReservaDialogues[];
extern Dialogue ato3OfertaDialogues[];

extern Dialogue ato3IaLeDialogues[];            // cenario 1: a IA le o papel (final ruim)
extern Dialogue ato3IaLeIaEscreveuDialogues[];  // cenario 1, se a IA escreveu o discurso no Ato 2
extern Dialogue ato3AjudaDialogues[];           // cenario 2: tirar uma opcao errada
extern Dialogue ato3SozinhaDialogues[];         // cenario 3: tentar sozinha
extern Dialogue ato3ConclusaoDialogues[];       // depois do desafio
extern Dialogue ato3ConclusaoIaEscreveuDialogues[];
extern Dialogue ato3EtapaConcluidaDialogues[];  // a barra ainda tem token: epilogo

int GetAto3AberturaCount(void);
int GetAto3ReservaCount(void);
int GetAto3OfertaCount(void);
int GetAto3IaLeCount(void);
int GetAto3IaLeIaEscreveuCount(void);
int GetAto3AjudaCount(void);
int GetAto3SozinhaCount(void);
int GetAto3ConclusaoCount(void);
int GetAto3ConclusaoIaEscreveuCount(void);
int GetAto3EtapaConcluidaCount(void);

// Escolha do Ato 3 (pop-up do MSN no PC do pulpito)
#define ATO3_ESCOLHA_PERGUNTA "O que você responde à IA?"
#define ATO3_ESCOLHA_A "IA lê o papel"
#define ATO3_ESCOLHA_B "Tirar uma opção errada por palavra"
#define ATO3_ESCOLHA_C "Tentar sozinha"

// Final ruim (tela do MSN offline e cartao)
#define ATO3_MSN_OFFLINE      "Tokens insuficientes. Retorno indisponível."
#define ATO3_CARTAO_IA_LEU    "A IA É UM APOIO. A VOZ CONTINUA SENDO SUA."
#define ATO3_CARTAO_SEM_TOKEN "PEDIR AJUDA EM TUDO TAMBÉM É NÃO DECIDIR."

#endif
