#ifndef ATO2_CORRIDA_H
#define ATO2_CORRIDA_H

#include "minigame.h"   // MinigameResult (CONTINUA, MENU, VENCEU)


// Desafio 2 do Ato 2: corrida de obstaculos no corredor da escola.
// Sem puzzle de tiros. Termina sozinho depois de alguns segundos.
// Sem game over: bater num obstaculo faz a Lia tropecar e perder um pouco de progresso.

// Comeca (ou recomeca) a corrida e carrega as imagens na primeira vez.
void InitAto2Corrida(void);

// Atualiza a corrida (uma vez por quadro).
// MINIGAME_VENCEU quando a Lia chega ao fim do corredor.
MinigameResult UpdateAto2Corrida(void);

// Desenha a corrida (entre BeginDrawing e EndDrawing).
void DrawAto2Corrida(void);

// Libera as imagens (chamar ao fechar o jogo).
void UnloadAto2Corrida(void);

#endif
