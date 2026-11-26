#ifndef FINAL_TOKENS_H
#define FINAL_TOKENS_H

#include "assets.h"

typedef enum {
    FINAL_TOKENS_CONTINUA,
    FINAL_TOKENS_REJOGAR,
    FINAL_TOKENS_MENU,
    FINAL_TOKENS_SAIR
} FinalTokensAcao;

// Chamado apenas depois de concluir o discurso e confirmar a barra em zero.
void FinalTokens_Iniciar(void);
FinalTokensAcao FinalTokens_Atualizar(void);
void FinalTokens_Desenhar(const Assets *assets);

#endif
