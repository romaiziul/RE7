#ifndef STORY_ATO2_H
#define STORY_ATO2_H

#include "scenes.h"

// Falas do Ato 2. Cada cena tem a lista de falas e a quantidade.
extern Dialogue ato2QuartoDialogues[];
int GetAto2QuartoCount(void);
extern Dialogue ato2CorredorDialogues[];
int GetAto2CorredorCount(void);
extern Dialogue ato2DeterminadaDialogues[];
int GetAto2DeterminadaCount(void);
extern Dialogue ato2EncontroDialogues[];
int GetAto2EncontroCount(void);
extern Dialogue ato2Res21Dialogues[];
int GetAto2Res21Count(void);
extern Dialogue ato2Res22Dialogues[];
int GetAto2Res22Count(void);
extern Dialogue ato2LanhouseDialogues[];
int GetAto2LanhouseCount(void);
extern Dialogue ato2LanhouseSemDialogues[];
int GetAto2LanhouseSemCount(void);
extern Dialogue ato2IAEscreveDialogues[];
int GetAto2IAEscreveCount(void);
extern Dialogue ato2UmaIdeiaDialogues[];
int GetAto2UmaIdeiaCount(void);
extern Dialogue ato2ComJoaoDialogues[];
int GetAto2ComJoaoCount(void);
extern Dialogue ato2C4Dialogues[];
int GetAto2C4Count(void);
extern Dialogue ato2C4ZeroDialogues[];
int GetAto2C4ZeroCount(void);
extern Dialogue ato2CgEscrevemDialogues[];
int GetAto2CgEscrevemCount(void);
extern Dialogue ato2CgEnsaioDialogues[];
int GetAto2CgEnsaioCount(void);
extern Dialogue ato2CgCasaNoiteDialogues[];
int GetAto2CgCasaNoiteCount(void);
extern Dialogue ato2CgFrenteEscolaDialogues[];
int GetAto2CgFrenteEscolaCount(void);
extern Dialogue ato2CgProfessoraDialogues[];
int GetAto2CgProfessoraCount(void);
extern Dialogue ato2CgProfessoraSemIADialogues[];
int GetAto2CgProfessoraSemIACount(void);

// Textos das escolhas
#define ATO2_ESCOLHA1_PERGUNTA "O que você faz?"
#define ATO2_ESCOLHA1_A "Tentar alcançar o João"
#define ATO2_ESCOLHA1_B "Não preciso de amigos, a IA é o suficiente."

#define ATO2_ESCOLHA2_PERGUNTA "O que você responde à IA?"
#define ATO2_ESCOLHA2_A "IA escreve o discurso"
#define ATO2_ESCOLHA2_B "Só uma ideia de abertura"
#define ATO2_ESCOLHA2_C "Fazer com a ajuda do João"

#endif
