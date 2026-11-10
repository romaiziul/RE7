#ifndef ATO3_H
#define ATO3_H

#include "raylib.h"
#include "assets.h"
#include <stdbool.h>

// Como o auditorio aparece num quadro.
typedef struct
{
    bool      pcOnline;     // PC do pulpito com o MSN aberto (false: apagado)
    bool      pcPiscando;   // o PC pisca e liga sozinho (token de reserva)
    bool      aplausos;     // a plateia aplaude (luz e brilhos subindo)
    float     foco;         // 0 a 1: a camera se aproxima da Lia
    float     nervosismo;   // 0 a 1: a Lia treme no pulpito
    Texture2D lia;          // expressao da Lia no pulpito (sprite lia07)
} Ato3Cena;

// Auditorio pintado (assets/ato3), com a Lia atras do pulpito.
void Ato3_DesenharCenario(const Ato3Cena *cena, const Assets *assets);

// Papel do discurso. nitidez[i] vai de 0 (palavra borrada) a 1 (nitida).
// destaque: palavra da etapa atual (-1 nenhuma), marcada com um sublinhado.
// folha: textura do papel (se nao carregar, desenha uma folha lisa).
void Ato3_DesenharPapel(
    Rectangle area,
    int tamanhoFonte,
    const float nitidez[3],
    int destaque,
    Vector2 tremor,
    Texture2D folha
);

#endif
