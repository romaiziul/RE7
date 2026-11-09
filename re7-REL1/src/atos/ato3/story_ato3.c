#include "story_ato3.h"

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

int GetAto3AberturaCount(void)
{
    return (int)(
        sizeof(ato3AberturaDialogues) /
        sizeof(ato3AberturaDialogues[0])
    );
}

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

int GetAto3ReservaCount(void)
{
    return (int)(sizeof(ato3ReservaDialogues) / sizeof(ato3ReservaDialogues[0]));
}

int GetAto3OfertaCount(void)
{
    return (int)(sizeof(ato3OfertaDialogues) / sizeof(ato3OfertaDialogues[0]));
}
