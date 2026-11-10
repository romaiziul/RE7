#include "story_ato3.h"

#define CONTAR(lista) ((int)(sizeof(lista) / sizeof((lista)[0])))

// O discurso completo e longo para a caixa de dialogo (3 linhas):
// vai em duas falas seguidas da Lia.
#define DISCURSO_PARTE1 \
    "Hoje é a primeira vez em muito tempo que falo em público, sabia? " \
    "Então estou bem nervosa..."
#define DISCURSO_PARTE2 \
    "...mas pude contar com a ajuda de meus amigos para escrever esse texto, " \
    "então me sinto bem mais confiante."


// =====================================================
// ABERTURA, RESERVA E OFERTA (comuns a todos os caminhos)
// =====================================================

Dialogue ato3AberturaDialogues[] =
{
    {
        "Professora",
        "Próxima participante: LIA.",
        POSE_PADRAO
    },
    {
        "Lia",
        "Eu não consigo ler nada. As letras estão se mexendo.",
        POSE_PREOCUPADA
    }
};

Dialogue ato3ReservaDialogues[] =
{
    { "IA", "Bateria de reserva: 1 token. É o último.", POSE_PADRAO }
};

Dialogue ato3OfertaDialogues[] =
{
    {
        "IA",
        "Posso ler o papel por você, tirar uma opção errada de cada palavra ou deixar você tentar sozinha.",
        POSE_PADRAO
    }
};


// =====================================================
// CENARIO 1: A IA LE O PAPEL (final ruim)
// =====================================================

Dialogue ato3IaLeDialogues[] =
{
    { "Lia", "Lê pra mim.", POSE_PADRAO },
    { "IA",  "Público. Amigos. Confiante.", POSE_PADRAO },
    { "Lia", DISCURSO_PARTE1, POSE_PADRAO },
    { "Lia", DISCURSO_PARTE2, POSE_PADRAO },
    { "Lia", "(baixinho) Eu nem li o que a gente escreveu.", POSE_TRISTE }
};

Dialogue ato3IaLeIaEscreveuDialogues[] =
{
    { "Lia", "Lê pra mim.", POSE_PADRAO },
    { "IA",  "Público. Amigos. Confiante.", POSE_PADRAO },
    { "Lia", DISCURSO_PARTE1, POSE_PADRAO },
    { "Lia", DISCURSO_PARTE2, POSE_PADRAO },
    { "Lia", "(baixinho) Eu nem sei o que está escrito aqui.", POSE_TRISTE }
};


// =====================================================
// CENARIOS 2 E 3: antes do desafio
// =====================================================

Dialogue ato3AjudaDialogues[] =
{
    { "Lia", "Me dá uma ajudinha. O resto eu consigo desenrolar.", POSE_DETERMINADA },
    { "IA",  "Feito. A escolha é sua.", POSE_PADRAO }
};

Dialogue ato3SozinhaDialogues[] =
{
    { "Lia", "Eu tento sozinha.", POSE_DETERMINADA },
    { "IA",  "Entendido.", POSE_PADRAO }
};


// =====================================================
// DEPOIS DO DESAFIO
// =====================================================

Dialogue ato3ConclusaoDialogues[] =
{
    {
        "Lia",
        "(respira fundo) Foi isso que a gente escreveu junto. Eu tinha esquecido que aceitar ajuda não é deixar fazerem por você.",
        POSE_FELIZ
    }
};

Dialogue ato3ConclusaoIaEscreveuDialogues[] =
{
    {
        "Lia",
        "(respira fundo) Não fui eu que escrevi, mas agora sou eu que estou falando.",
        POSE_DETERMINADA
    }
};

Dialogue ato3EtapaConcluidaDialogues[] =
{
    { "IA", "Etapa concluída.", POSE_PADRAO }
};


// =====================================================
// CONTAGENS
// =====================================================

int GetAto3AberturaCount(void)            { return CONTAR(ato3AberturaDialogues); }
int GetAto3ReservaCount(void)             { return CONTAR(ato3ReservaDialogues); }
int GetAto3OfertaCount(void)              { return CONTAR(ato3OfertaDialogues); }
int GetAto3IaLeCount(void)                { return CONTAR(ato3IaLeDialogues); }
int GetAto3IaLeIaEscreveuCount(void)      { return CONTAR(ato3IaLeIaEscreveuDialogues); }
int GetAto3AjudaCount(void)               { return CONTAR(ato3AjudaDialogues); }
int GetAto3SozinhaCount(void)             { return CONTAR(ato3SozinhaDialogues); }
int GetAto3ConclusaoCount(void)           { return CONTAR(ato3ConclusaoDialogues); }
int GetAto3ConclusaoIaEscreveuCount(void) { return CONTAR(ato3ConclusaoIaEscreveuDialogues); }
int GetAto3EtapaConcluidaCount(void)      { return CONTAR(ato3EtapaConcluidaDialogues); }
