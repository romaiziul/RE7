#ifndef ORATORIA_TYPING_H
#define ORATORIA_TYPING_H

#include "ato3_logica.h"

#define ORATORIA_MAX_LETRAS 32
#define ORATORIA_TEMPO_MINIMO 7.0f
#define ORATORIA_TEMPO_MAXIMO 10.0f
#define ORATORIA_PENALIDADE 0.35f

typedef enum {
    ORATORIA_IGNORADA, ORATORIA_LETRA_CERTA, ORATORIA_LETRA_ERRADA,
    ORATORIA_PALAVRA_CERTA, ORATORIA_PALAVRA_ERRADA
} OratoriaEntrada;

typedef struct {
    EntreLinhas discurso;
    bool digitando, esgotou;
    int opcao, tamanho, progresso, erros;
    int letras[ORATORIA_MAX_LETRAS]; // codepoints, preservando os acentos
    int ordem[ORATORIA_MAX_LETRAS];  // indices unicos: letras repetidas sao distintas
    float limite, restante;
    unsigned aleatorio;
} Oratoria;

void Oratoria_Iniciar(Oratoria *jogo, Ato3Ajuda modo, unsigned semente);
bool Oratoria_Selecionar(Oratoria *jogo, int opcao);
OratoriaEntrada Oratoria_Digitar(Oratoria *jogo, int codepoint);
void Oratoria_Atualizar(Oratoria *jogo, float dt);
bool Oratoria_Avancar(Oratoria *jogo);

#endif
