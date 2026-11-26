#ifndef FINAL_DISCURSO_H
#define FINAL_DISCURSO_H

#include "assets.h"

typedef enum {
    FINAL_DISCURSO_CONTINUA,
    FINAL_DISCURSO_REJOGAR,
    FINAL_DISCURSO_MENU,
    FINAL_DISCURSO_SAIR
} FinalDiscursoAcao;

void FinalDiscurso_Iniciar(void);
FinalDiscursoAcao FinalDiscurso_Atualizar(void);
void FinalDiscurso_Desenhar(const Assets *assets);

#endif
