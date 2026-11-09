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
    SCENE_ATO2_QUARTO,        // cena 1: quarto em 2007, a barra de tokens aparece
    SCENE_ATO2_CORREDOR,      // cena 2: corredor da escola (depois: escolha 1)
    SCENE_ATO2_DETERMINADA,   // escolha 1, "tentar alcancar": Lia determinada (depois: corrida)
    SCENE_ATO2_LANHOUSE,      // cena 3.1: lan house com o Joao (depois: escolha 2)
    SCENE_ATO2_IA_ESCREVE,    // escolha 2: IA escreve o discurso (zera a barra)
    SCENE_ATO2_UMA_IDEIA,     // escolha 2: so uma ideia (-1)
    SCENE_ATO2_COM_JOAO,      // escolha 2: fazer com a ajuda do Joao (+1)
    SCENE_ATO2_C4,            // cena 4, barra em 1 a 3
    SCENE_ATO2_CG_ESCREVEM,   // imagem: treinando com o Joao
    SCENE_ATO2_CG_PROFESSORA, // imagem: a professora corrigindo (com "Etapa concluida")
    SCENE_ATO2_C4_ZERO,       // cena 4, barra em 0
    SCENE_ATO2_CG_ENSAIO,     // imagem: ensaiando
    SCENE_ATO2_ENCONTRO,      // depois da corrida: a Lia alcanca o Joao
    SCENE_ATO2_RES21,         // resultado 2.1: corredor, o Joao convida para a lan house
    SCENE_ATO2_RES22,         // resultado 2.2: corredor, a Lia ignorou o Joao
    SCENE_ATO2_LANHOUSE_SEM,  // cena 3.2: lan house sem o Joao (depois: escolha 2)
    SCENE_ATO2_CG_PROFESSORA_SEM_IA, // imagem: a professora corrigindo, sem a IA
    SCENE_ATO2_CG_CASA_NOITE, // imagem provisoria: treinando em casa, de noite
    SCENE_ATO2_CG_FRENTE_ESCOLA, // imagem provisoria: frente da escola, de dia

    // ---- Epilogo (provisorio: o Ato 3 ainda nao foi feito) ----
    SCENE_EPILOGUE,

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
    POSE_FELIZ,
    POSE_TRISTE,        // so a Lia 2007
    POSE_PENSATIVA,     // so a Lia 2007
    POSE_DETERMINADA    // so a Lia 2007

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
