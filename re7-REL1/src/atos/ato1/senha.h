// senha.h
// Minijogo da senha do PC (Ato 1, escolha A1-E1).
// Duas etapas: na etapa 1, a jogadora escolhe os quatro caracteres que respondem
// as pistas dos fragmentos (Kelly Key, Year, 2007); na etapa 2, arrasta os
// caracteres para a ordem certa usando pistas de posicao. A senha e Y2K7.
// O visual e uma tela de login inspirada no Windows XP ("Doors XP").
// A IA so oferece ajuda na etapa 2, a cada SENHA_ERROS_RODADA erros.
#ifndef SENHA_H
#define SENHA_H

#include <stdbool.h>

#define SENHA_CORRETA        "Y2K7"
#define SENHA_TAMANHO        4
#define SENHA_ERROS_RODADA   3   // a cada 3 erros na etapa 2, a IA oferece ajuda (o texto do pop-up diz "tres vezes")

void Senha_ComecarJogo(void);  // novo jogo: zera tudo, inclusive a ajuda ja usada
void Senha_Iniciar(void);      // quando a LIA senta no PC
void Senha_Atualizar(void);    // uma vez por quadro
void Senha_Desenhar(void);     // entre BeginDrawing e EndDrawing

bool Senha_Concluida(void);    // acertou: segue para a Cena 3
void Senha_Descarregar(void);  // ao fechar o jogo: libera as imagens da tela de login

#endif
