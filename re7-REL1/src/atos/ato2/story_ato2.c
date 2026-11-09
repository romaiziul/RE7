#include "story_ato2.h"

// Falas do Ato 2, seguindo o roteiro (Documentacao > ATO 2).
// Nomes na tela: "Lia", "IA" e "João" (o amigo).

#define CONTA(v) ((int)(sizeof(v) / sizeof((v)[0])))


// CENA 1: QUARTO EM 2007 (o PC liga sozinho)

Dialogue ato2QuartoDialogues[] =
{
    { "Lia", "Todo esse trabalho pra isso? Eu quero voltar e fazer a minha palestra real!", POSE_BRAVA },
    { "IA",  "Você volta, mas só se vencer os desafios daqui." },
    { "IA",  "Você não está só! Mas tenha cuidado..." },
    { "IA",  "A barra de tokens mostra quanto de ajuda eu ainda posso dar. Se ela zerar, eu desligo e não consigo te levar de volta." },
    { "Lia", "Então vamos logo para a escola resolver isso.", POSE_DECIDIDA }
    // a barra de tokens aparece na tela; depois a tela escurece devagar
};


// CENA 2: CORREDOR DA ESCOLA

Dialogue ato2CorredorDialogues[] =
{
    { "Lia", "Assistir aulas foi estranhamente legal. Eu não esperava sentir tanta nostalgia.", POSE_SURPRESA },
    { "Lia", "Aquele é o... João?", POSE_SURPRESA },
    { "IA",  "Sim. O seu amigo João de quem você se afastou." },
    { "IA",  "Ele seria um aliado importante nos seus desafios..." }
    // depois: ESCOLHA 1 (nao usa a barra)
};

// Escolha 1, opcao "Tentar alcancar o Joao": imagem da Lia determinada (sem fala), depois a corrida
Dialogue ato2DeterminadaDialogues[] =
{
    { "", "" }
};

// Depois da corrida: a Lia alcanca o amigo no fim do corredor
// (ela feliz, ele surpreso; depois a tela escurece devagar e volta no corredor)
Dialogue ato2EncontroDialogues[] =
{
    { "Lia", "Ufa, consegui alcançá-lo!", POSE_RINDO }
};

// RESULTADO 2.1: corredor, depois de alcancar o Joao
Dialogue ato2Res21Dialogues[] =
{
    { "João", "Oi, Lia! Que bom encontrar você. A professora falou que você também se inscreveu no concurso de oratória!", POSE_FELIZ },
    { "Lia",  "Me inscrevi sim! O que você acha de...", POSE_CONFUSA },
    { "João", "A galera vai se encontrar na Lan House depois do almoço, bora jogar um CS e depois treinar um pouco?", POSE_FELIZ },
    { "Lia",  "Te encontro lá, João!", POSE_RINDO }
    // fade lento para preto e vai para a Cena 3.1
};

// RESULTADO 2.2: corredor, a Lia ignorou o Joao
Dialogue ato2Res22Dialogues[] =
{
    { "Lia", "Não preciso de amigos, a IA é o suficiente.", POSE_CONTRARIADA },
    { "Lia", "Você já me fez assistir uma manhã inteira de aulas... Pra onde vai me obrigar a ir agora?", POSE_CONTRARIADA },
    { "IA",  "Vamos para o seu lugar favorito de 2007: a Lan House..." }
    // fade e vai para a Cena 3.2
};


// CENA 3.1: LAN HOUSE (com o Joao)

Dialogue ato2LanhouseDialogues[] =
{
    { "Lia",  "Eu tinha esquecido como esse jogo era legal!", POSE_RINDO },
    { "João", "Foi mesmo... Agora que relaxamos podemos nos preparar para o discurso!", POSE_FELIZ },
    { "IA",   "Vejo que está se divertindo com seus amigos. Agora que precisa focar no concurso, como quer seguir?" }
    // aparece o pop-up do MSN; depois: ESCOLHA 2
};

// CENA 3.2: LAN HOUSE (sem o Joao). O Joao entra na segunda fala.
Dialogue ato2LanhouseSemDialogues[] =
{
    { "Lia",  "Eu tinha esquecido como esse jogo era legal!", POSE_RINDO },
    { "João", "Oi, Lia! Que bom encontrar você. A professora falou que você também se inscreveu no concurso de oratória!", POSE_FELIZ },
    { "Lia",  "Sim, me inscrevi!", POSE_RINDO },
    { "João", "E seu discurso já está pronto?", POSE_FELIZ },
    { "Lia",  "Ainda não...", POSE_CONFUSA },
    { "IA",   "Vejo que está se divertindo. Agora que precisa focar no concurso, como quer seguir?" }
    // depois: ESCOLHA 2
};


// ESCOLHA 2: resposta de cada opcao (iguais com ou sem o Joao)

// Opcao 1: IA escreve o discurso (zera a barra; a IA fica offline ate o fim do ato)
Dialogue ato2IAEscreveDialogues[] =
{
    { "IA",  "Tudo pronto, mas... parece que está sozinha daqui pra frente." },
    { "Lia", "Não!", POSE_SURPRESA }
};

// Opcao 2: so uma ideia de abertura (-1 token)
Dialogue ato2UmaIdeiaDialogues[] =
{
    { "Lia", "Só uma ideia pra começar.", POSE_CONFUSA },
    { "IA",  "Abra com uma pergunta pra plateia. O resto é seu." },
    { "Lia", "Boa ideia... Já sei como devo começar...", POSE_RINDO }
};

// Opcao 3: fazer com a ajuda do Joao (+1 token, maximo 3)
Dialogue ato2ComJoaoDialogues[] =
{
    { "IA",  "Boa ideia! João é muito bom em oratória..." },
    { "Lia", "João, ainda não sei por onde começar. Pode me ajudar?", POSE_RINDO }
};


// CENA 4: O DISCURSO (a mesma com ou sem a corrida; o Joao esta na lan house nos dois casos)

// Barra em 1, 2 ou 3
Dialogue ato2C4Dialogues[] =
{
    { "João", "Bora escrever o discurso? A gente faz no papel mesmo, do nosso jeito.", POSE_FELIZ },
    { "Lia",  "Bora!", POSE_RINDO }
};

// Barra em 0 (a IA escreveu o discurso). A IA esta offline.
Dialogue ato2C4ZeroDialogues[] =
{
    { "João", "Então?", POSE_FELIZ },
    { "Lia",  "Na verdade eu já tenho discurso pronto...", POSE_CONFUSA },
    { "João", "Já tem o discurso pronto? Então bora ensaiar.", POSE_FELIZ }
};

// Imagem (sem fala): a Lia treinando com o amigo na lan house.
Dialogue ato2CgEscrevemDialogues[] =
{
    { "", "" }
};

// Imagem (sem fala): a Lia ensaiando com o papel na mao e o amigo ouvindo.
Dialogue ato2CgEnsaioDialogues[] =
{
    { "", "" }
};

// IMAGEM PROVISORIA (desenhada pelo jogo): a Lia treina o discurso em casa, de noite.
Dialogue ato2CgCasaNoiteDialogues[] =
{
    { "", "" }
};

// IMAGEM PROVISORIA (desenhada pelo jogo): de dia, na frente da escola.
Dialogue ato2CgFrenteEscolaDialogues[] =
{
    { "", "" }
};

// Imagem: a professora corrigindo, e a IA na tela do jogo (barra em 1 a 3)
Dialogue ato2CgProfessoraDialogues[] =
{
    { "", "" },
    { "IA", "Etapa concluída." }
};

// Imagem: a professora corrigindo, sem fala da IA (ela esta offline, barra em 0)
Dialogue ato2CgProfessoraSemIADialogues[] =
{
    { "", "" }
};


// CONTAGEM DE DIALOGOS

int GetAto2QuartoCount(void)             { return CONTA(ato2QuartoDialogues); }
int GetAto2CorredorCount(void)           { return CONTA(ato2CorredorDialogues); }
int GetAto2DeterminadaCount(void)        { return CONTA(ato2DeterminadaDialogues); }
int GetAto2EncontroCount(void)           { return CONTA(ato2EncontroDialogues); }
int GetAto2Res21Count(void)              { return CONTA(ato2Res21Dialogues); }
int GetAto2Res22Count(void)              { return CONTA(ato2Res22Dialogues); }
int GetAto2LanhouseCount(void)           { return CONTA(ato2LanhouseDialogues); }
int GetAto2LanhouseSemCount(void)        { return CONTA(ato2LanhouseSemDialogues); }
int GetAto2IAEscreveCount(void)          { return CONTA(ato2IAEscreveDialogues); }
int GetAto2UmaIdeiaCount(void)           { return CONTA(ato2UmaIdeiaDialogues); }
int GetAto2ComJoaoCount(void)            { return CONTA(ato2ComJoaoDialogues); }
int GetAto2C4Count(void)                 { return CONTA(ato2C4Dialogues); }
int GetAto2C4ZeroCount(void)             { return CONTA(ato2C4ZeroDialogues); }
int GetAto2CgEscrevemCount(void)         { return CONTA(ato2CgEscrevemDialogues); }
int GetAto2CgEnsaioCount(void)           { return CONTA(ato2CgEnsaioDialogues); }
int GetAto2CgCasaNoiteCount(void)        { return CONTA(ato2CgCasaNoiteDialogues); }
int GetAto2CgFrenteEscolaCount(void)     { return CONTA(ato2CgFrenteEscolaDialogues); }
int GetAto2CgProfessoraCount(void)       { return CONTA(ato2CgProfessoraDialogues); }
int GetAto2CgProfessoraSemIACount(void)  { return CONTA(ato2CgProfessoraSemIADialogues); }
