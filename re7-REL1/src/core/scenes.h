#ifndef SCENES_H
#define SCENES_H


// Todas as cenas do jogo.
// Cada cena e um trecho de dialogo. O que acontece no fim de cada uma
// (escolha, minijogo, proxima cena) esta no game.c.

typedef enum
{
    // ---- Ato 1 ----
    SCENE_PRESENT,          // quarto em 2026, conversa com a IA
    SCENE_2007,             // acorda no quarto em 2007 (depois: explorar o quarto)
    SCENE_2007_PC,          // PC bloqueado (depois: minijogo da senha)
    SCENE_ATO1_IA_A,        // a IA explica (depois: escolha do ano)
    SCENE_ATO1_IA_B,        // depois do ano certo

    // ---- Ato 2 ----
    SCENE_ATO2_QUARTO,      // quarto em 2007, a barra de tokens aparece
    SCENE_ATO2_CORREDOR,    // corredor da escola (depois: escolha do amigo)
    SCENE_ATO2_POSTER,      // poster do concurso (depois: corrida)
    SCENE_ATO2_LANHOUSE,    // lan house (depois: escolha da IA)
    SCENE_ATO2_IA_ESCREVE,  // escolha: IA escreve o discurso (zera a barra)
    SCENE_ATO2_UMA_IDEIA,   // escolha: so uma ideia (-1)
    SCENE_ATO2_FECHAR,      // escolha: fechar a janela (+1)
    SCENE_ATO2_DISCURSO,    // barra em 1 a 3: escrever junto
    SCENE_ATO2_CG_ESCREVEM, // imagem: escrevendo o discurso
    SCENE_ATO2_CG_PROFESSORA, // imagem: a professora corrigindo
    SCENE_ATO2_ENSAIO,      // barra em 0: ensaiar
    SCENE_ATO2_CG_ENSAIO,   // imagem: ensaiando

    // ---- Epilogo (provisorio: o Ato 3 ainda nao foi feito) ----
    SCENE_EPILOGUE,

    // Ato 3.
    SCENE_ATO3_ABERTURA,
    SCENE_ATO3_RESERVA,
    SCENE_ATO3_OFERTA,

    SCENE_COUNT 

} SceneID;


// Expressao da personagem na fala.
// POSE_PADRAO e a expressao neutra. A Lia (2026 ou 2007) e o amigo usam a que existir.

typedef enum
{
    POSE_PADRAO = 0,
    POSE_CONFUSA,
    POSE_BRAVA,
    POSE_SURPRESA,
    POSE_CONTRARIADA,
    POSE_NOSTALGICA,
    POSE_RINDO,
    POSE_PREOCUPADA,
    POSE_DECIDIDA,
    POSE_FELIZ

} Pose;


// Uma fala
// (se "pose" for omitido na lista, vale POSE_PADRAO)

typedef struct
{
    const char *speaker;

    const char *text;

    Pose pose;

} Dialogue;


// Dados de uma cena

typedef struct
{
    Dialogue *dialogues;

    int dialogueCount;

    SceneID nextScene;

} Scene;


// Retorna os dados da cena

Scene GetScene(SceneID id);


#endif
