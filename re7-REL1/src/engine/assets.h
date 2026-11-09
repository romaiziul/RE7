#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"


typedef struct
{

    // backgrounds

    Texture2D bgPresent;

    Texture2D bg2007;

    Texture2D bgRua;


    // botões do menu: [botão][estado]
    // botão: 0 novo jogo, 1 continuar, 2 créditos, 3 sair
    // estado: 0 normal, 1 hover, 2 pressed

    Texture2D menuButtons[4][3];

    Texture2D logo;

    // avatar da IA (estrela do logo)

    Texture2D iaStar;


    // personagens

    Texture2D liaJovemLado;     // = lia07[0] (mesma imagem, nao descarregar duas vezes)
    Texture2D liaFrente;        // = lia26[0]

    // Lia adulta (2026): 0 neutra, 1 preocupada, 2 decidida
    Texture2D lia26[3];

    // Lia adolescente (2007): 0 neutra, 1 confusa, 2 brava, 3 surpresa,
    // 4 contrariada, 5 nostalgica, 6 rindo, 7 feliz, 8 triste, 9 pensativa, 10 determinada
    Texture2D lia07[11];

    // amigo: 0 neutro, 1 feliz
    Texture2D amigo[2];

    // amigo surpreso (encontro depois da corrida): 0 olhando para a esquerda, 1 olhando para a direita
    Texture2D amigoSurpreso[2];


    // Ato 1 e Ato 2: cenarios e imagens de cena

    Texture2D cgTela2026;       // rosto da Lia iluminado pela tela (Ato 1, cena 1)
    Texture2D bgCorredor;       // corredor da escola (Ato 2)
    Texture2D bgLanhouse;       // lan house (Ato 2)
    Texture2D poster;           // poster do concurso de oratoria (Ato 2)
    Texture2D cgEscrevem;       // Lia e o amigo escrevendo o discurso
    Texture2D cgProfessora;     // a professora corrigindo
    Texture2D cgEnsaio;         // Lia ensaiando
    Texture2D cgCasaNoite;      // Lia treinando o discurso em casa, de noite
    Texture2D cgFrenteEscola;   // frente da escola, de dia
    Texture2D pcBoot;           // tela de boot do Doors XP (fim do Ato 1)
    Texture2D pcDesktop;        // area de trabalho do Doors XP
    Texture2D pcPerfil;         // pagina da rede social com o recado da Lia

} Assets;


void LoadAssets(Assets *assets);

void UnloadAssets(Assets *assets);


#endif