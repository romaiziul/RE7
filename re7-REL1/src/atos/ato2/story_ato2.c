#include "story_ato2.h"

// Falas do Ato 2, seguindo o roteiro (Documentacao > ATO 2).

#define CONTA(v) ((int)(sizeof(v) / sizeof((v)[0])))


// CENA 1: QUARTO EM 2007 (o PC liga sozinho)

Dialogue ato2QuartoDialogues[] =
{
    { "Lia", "Eu não quero ir pra escola. Eu quero voltar e fazer a minha palestra!", POSE_CONTRARIADA },
    { "IA",  "Você volta, mas só se vencer os desafios daqui. A barra de tokens mostra quanto de ajuda eu ainda posso dar. Se ela zerar, eu desligo e não consigo te levar de volta." }
    // a barra de tokens aparece na tela
};


// CENA 2: CORREDOR DA ESCOLA

Dialogue ato2CorredorDialogues[] =
{
    { "Amigo", "E aí, Lia!", POSE_FELIZ },
    { "Lia",   "Faz tanto tempo... A gente não se fala desde... Aquele dia...", POSE_NOSTALGICA }
    // depois: ESCOLHA 1 (nao usa a barra). Se ignorar, a escolha volta.
};

// Depois de dar uma chance ao amigo
Dialogue ato2PosterDialogues[] =
{
    { "Amigo", "Viu o pôster do concurso de oratória? A professora queria que você se inscrevesse.", POSE_FELIZ },
    // aqui o poster fica na tela (hotspot)
    { "Lia",   "Eu não lembrava disso. Será que isso muda alguma coisa lá na frente?" },
    { "Amigo", "Eu te ajudo a ensaiar. Mas antes passa na lan house, a galera tá lá.", POSE_FELIZ }
    // depois: DESAFIO 2 (obstaculos no corredor, sem ajuda da IA)
};


// CENA 3: LAN HOUSE

Dialogue ato2LanhouseDialogues[] =
{
    { "Lia", "Eu tinha esquecido como esse jogo era legal!", POSE_RINDO },
    // aparece o pop-up do MSN
    { "IA",  "Vejo que está se divertindo com seus amigos. Posso escrever o discurso do concurso agora." }
    // depois: ESCOLHA 2
};

// Opcao 1: IA escreve o discurso (zera a barra; a IA fica offline ate o fim do ato)
Dialogue ato2IAEscreveDialogues[] =
{
    { "Lia", "Pode escrever." },
    { "IA",  "Pronto." }
};

// Opcao 2: so uma ideia de abertura (-1 token)
Dialogue ato2UmaIdeiaDialogues[] =
{
    { "Lia", "Só uma ideia pra começar." },
    { "IA",  "Abra com uma pergunta pra plateia. O resto é seu." }
};

// Opcao 3: fechar a janela (+1 token, maximo 3)
Dialogue ato2FecharJanelaDialogues[] =
{
    { "Lia", "Deixa eu jogar um pouco!", POSE_RINDO }
};


// CENA 4: O DISCURSO

// Barra em 1, 2 ou 3
Dialogue ato2DiscursoDialogues[] =
{
    { "Amigo", "Bora escrever o discurso? A gente faz no papel mesmo, do nosso jeito.", POSE_FELIZ }
};

// Imagens (sem fala): a Lia recebendo ajuda do amigo e, depois, a professora corrigindo.
Dialogue ato2CgEscrevemDialogues[] =
{
    { "", "" }
};

Dialogue ato2CgProfessoraDialogues[] =
{
    { "", "" },
    { "IA", "Etapa concluída." }
};

// Barra em 0 (a IA escreveu o discurso). A IA esta offline e nao fala.
Dialogue ato2EnsaioDialogues[] =
{
    { "Amigo", "Já tem o discurso pronto? Então bora ensaiar.", POSE_FELIZ }
};

// Imagem (sem fala): a Lia ensaiando com o papel na mao e o amigo ouvindo.
Dialogue ato2CgEnsaioDialogues[] =
{
    { "", "" }
};


// CONTAGEM DE DIALOGOS

int GetAto2QuartoCount(void)        { return CONTA(ato2QuartoDialogues); }
int GetAto2CorredorCount(void)      { return CONTA(ato2CorredorDialogues); }
int GetAto2PosterCount(void)        { return CONTA(ato2PosterDialogues); }
int GetAto2LanhouseCount(void)      { return CONTA(ato2LanhouseDialogues); }
int GetAto2IAEscreveCount(void)     { return CONTA(ato2IAEscreveDialogues); }
int GetAto2UmaIdeiaCount(void)      { return CONTA(ato2UmaIdeiaDialogues); }
int GetAto2FecharJanelaCount(void)  { return CONTA(ato2FecharJanelaDialogues); }
int GetAto2DiscursoCount(void)      { return CONTA(ato2DiscursoDialogues); }
int GetAto2CgEscrevemCount(void)    { return CONTA(ato2CgEscrevemDialogues); }
int GetAto2CgProfessoraCount(void)  { return CONTA(ato2CgProfessoraDialogues); }
int GetAto2EnsaioCount(void)        { return CONTA(ato2EnsaioDialogues); }
int GetAto2CgEnsaioCount(void)      { return CONTA(ato2CgEnsaioDialogues); }
