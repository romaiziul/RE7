#ifndef STORY_ATO2_H
#define STORY_ATO2_H

#include "scenes.h"

// Cena 1: quarto em 2007
extern Dialogue ato2QuartoDialogues[];
int GetAto2QuartoCount(void);

// Cena 2: corredor (antes da escolha 1)
extern Dialogue ato2CorredorDialogues[];
int GetAto2CorredorCount(void);

// Cena 2: depois da escolha 1 (poster e convite para a lan house)
extern Dialogue ato2PosterDialogues[];
int GetAto2PosterCount(void);

// Cena 3: lan house (antes da escolha 2)
extern Dialogue ato2LanhouseDialogues[];
int GetAto2LanhouseCount(void);

// Cena 3: resposta de cada opcao da escolha 2
extern Dialogue ato2IAEscreveDialogues[];     // zera a barra
int GetAto2IAEscreveCount(void);
extern Dialogue ato2UmaIdeiaDialogues[];      // -1 token
int GetAto2UmaIdeiaCount(void);
extern Dialogue ato2FecharJanelaDialogues[];  // +1 token (maximo 3)
int GetAto2FecharJanelaCount(void);

// Cena 4: discurso (barra em 1, 2 ou 3)
extern Dialogue ato2DiscursoDialogues[];
int GetAto2DiscursoCount(void);
extern Dialogue ato2CgEscrevemDialogues[];
int GetAto2CgEscrevemCount(void);
extern Dialogue ato2CgProfessoraDialogues[];
int GetAto2CgProfessoraCount(void);

// Cena 4: ensaio (barra em 0, a IA escreveu o discurso)
extern Dialogue ato2EnsaioDialogues[];
int GetAto2EnsaioCount(void);
extern Dialogue ato2CgEnsaioDialogues[];
int GetAto2CgEnsaioCount(void);

// Textos das escolhas
#define ATO2_ESCOLHA1_PERGUNTA "O que você faz?"
#define ATO2_ESCOLHA1_A "Dar uma chance ao amigo"
#define ATO2_ESCOLHA1_B "Ignorar e seguir"

#define ATO2_ESCOLHA2_PERGUNTA "O que você responde à IA?"
#define ATO2_ESCOLHA2_A "IA escreve o discurso"
#define ATO2_ESCOLHA2_B "Só uma ideia de abertura"
#define ATO2_ESCOLHA2_C "Fechar a janela"

#endif
