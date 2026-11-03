#include "story.h"


// =====================================================
// CENA: EPILOGO (DE VOLTA AO PRESENTE)
// =====================================================

Dialogue epilogueDialogues[] =
{
    {
        "Lia",
        "Voltei... ainda e a noite antes da palestra."
    },

    {
        "Lia",
        "Eu nao tenho mais tanto medo. Sei por onde comecar."
    },

    {
        "Lia",
        "Vou escrever o discurso. Com a minha voz."
    },

    {
        "IA",
        "Estarei aqui. Para ajudar, nao para substituir voce."
    },

    {
        "Lia",
        "Obrigada por me ensinar isso."
    }
};


// =====================================================
// CONTAGEM DE DIÁLOGOS
// =====================================================

int GetEpilogueDialogueCount(void)
{
    return sizeof(epilogueDialogues) / sizeof(epilogueDialogues[0]);
}
