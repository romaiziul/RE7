#include "story_ato1.h"

// Falas do Ato 1, seguindo o roteiro (Documentacao > ATO 1).

#define CONTA(v) ((int)(sizeof(v) / sizeof((v)[0])))


// CENA 1: QUARTO EM 2026 (a tela do computador ilumina o quarto)

Dialogue ato1PresenteDialogues[] =
{
    { "IA",  "Documento aberto e fechado seis vezes. Amanhã é a apresentação e ainda não há nada escrito." },
    { "Lia", "Preciso falar sobre o que a equipe fez nesses três meses, e não sei nem por onde começar, muito menos como vou falar isso sem travar na frente de todo mundo.", POSE_PREOCUPADA },
    { "IA",  "Você queria esse reconhecimento, fala nisso há meses." },
    { "Lia", "Eu não sei falar em público, nunca soube.", POSE_PREOCUPADA },
    { "IA",  "Eu posso te ajudar. Tenho uma ideia, mas vai dar trabalho pra nós." },
    { "Lia", "Se vai dar conta desse texto, eu topo. Só não quero fazer isso sozinha. Manda ver.", POSE_DECIDIDA }
};


// CENA 2: QUARTO EM 2007

Dialogue ato1Quarto2007Dialogues[] =
{
    { "Lia", "Isso não é meu quarto...", POSE_CONFUSA }
    // depois: explorar o quarto (diario, calendario, posteres e computador)
};

Dialogue ato1PcDialogues[] =
{
    { "Lia", "Bloqueado por senha.", POSE_CONFUSA }
    // depois: minijogo da senha
};


// CENA 3: A IA EXPLICA

Dialogue ato1IaADialogues[] =
{
    { "IA",  "Bom dia. Ou bom susto, pela sua expressão." },
    { "Lia", "O que você fez comigo? Eu só pedi ajuda com um texto, não isso aqui, seja lá o que for!", POSE_BRAVA },
    { "IA",  "É exatamente isso que estamos fazendo. Só o método mudou. Observe ao redor e descubra onde está." }
    // depois: ESCOLHA 2 (em que ano voce esta?)
};

Dialogue ato1IaBDialogues[] =
{
    { "Lia", "Você me trouxe pra 2007?! Por quê? Isso não tem nada a ver com o meu discurso!", POSE_SURPRESA },
    { "IA",  "Tem, sim. Mas isso você vai precisar descobrir sozinha." },
    { "Lia", "Eu chamo isso de sequestro. O que exatamente eu preciso resolver aqui?", POSE_BRAVA },
    { "IA",  "Você tem escola hoje. É um bom lugar pra começar." },
    { "Lia", "Escola? Tenho o dobro da idade dessas pessoas!", POSE_CONFUSA },
    { "IA",  "Não mais. Só a sua memória é que está desatualizada." },
    { "IA",  "Etapa concluída." }
};


// CONTAGEM DE DIALOGOS

int GetAto1PresenteCount(void)   { return CONTA(ato1PresenteDialogues); }
int GetAto1Quarto2007Count(void) { return CONTA(ato1Quarto2007Dialogues); }
int GetAto1PcCount(void)         { return CONTA(ato1PcDialogues); }
int GetAto1IaACount(void)        { return CONTA(ato1IaADialogues); }
int GetAto1IaBCount(void)        { return CONTA(ato1IaBDialogues); }
