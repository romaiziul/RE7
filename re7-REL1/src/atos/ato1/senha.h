// senha.h
// Minijogo da senha do PC (Ato 1, escolha A1-E1).
// Logica proposicional: a jogadora descobre a senha Y2K7 a partir de
// quatro fragmentos que mostram so a pista, sem o caractere.
#ifndef SENHA_H
#define SENHA_H

#include <stdbool.h>

#define SENHA_CORRETA        "Y2K7"
#define SENHA_TAMANHO        4
#define SENHA_ERROS_RODADA   5   // no 5o erro de cada rodada, a IA oferece ajuda

void Senha_ComecarJogo(void);  // novo jogo: zera tudo (ajuda, cards, proposicoes)
void Senha_Iniciar(void);      // quando a LIA senta no PC
void Senha_Atualizar(void);    // uma vez por quadro
void Senha_Desenhar(void);     // entre BeginDrawing e EndDrawing

bool Senha_Concluida(void);    // acertou: segue para a Cena 3

#endif
