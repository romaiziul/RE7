#include "scenes.h"
#include "story.h"
#include "story_ato1.h"
#include "story_ato2.h"
#include "story_ato3.h"


// =====================================================
// RETORNAR CENA
//
// "nextScene" e a cena seguinte quando nao ha escolha nem minijogo
// no fim da cena. Os casos com escolha, minijogo, fade ou ramo (com/sem
// Joao, barra em 0) estao no game.c (funcao FimDaCena).
// =====================================================

Scene GetScene(SceneID id)
{
    switch (id)
    {
        // ---------------- Ato 1 ----------------

        case SCENE_PRESENT:
            return (Scene){ ato1PresenteDialogues, GetAto1PresenteCount(), SCENE_2007 };

        case SCENE_2007:
            return (Scene){ ato1Quarto2007Dialogues, GetAto1Quarto2007Count(), SCENE_2007_PC };

        case SCENE_2007_PC:
            return (Scene){ ato1PcDialogues, GetAto1PcCount(), SCENE_ATO1_IA_A };

        case SCENE_ATO1_IA_A:
            return (Scene){ ato1IaADialogues, GetAto1IaACount(), SCENE_ATO1_IA_B };

        case SCENE_ATO1_IA_B:
            return (Scene){ ato1IaBDialogues, GetAto1IaBCount(), SCENE_ATO2_QUARTO };


        // ---------------- Ato 2 ----------------

        case SCENE_ATO2_QUARTO:
            return (Scene){ ato2QuartoDialogues, GetAto2QuartoCount(), SCENE_ATO2_CORREDOR };

        case SCENE_ATO2_CORREDOR:
            return (Scene){ ato2CorredorDialogues, GetAto2CorredorCount(), SCENE_ATO2_DETERMINADA };

        case SCENE_ATO2_DETERMINADA:
            return (Scene){ ato2DeterminadaDialogues, GetAto2DeterminadaCount(), SCENE_ATO2_ENCONTRO };

        case SCENE_ATO2_ENCONTRO:
            return (Scene){ ato2EncontroDialogues, GetAto2EncontroCount(), SCENE_ATO2_RES21 };

        case SCENE_ATO2_RES21:
            return (Scene){ ato2Res21Dialogues, GetAto2Res21Count(), SCENE_ATO2_LANHOUSE };

        case SCENE_ATO2_RES22:
            return (Scene){ ato2Res22Dialogues, GetAto2Res22Count(), SCENE_ATO2_LANHOUSE_SEM };

        case SCENE_ATO2_LANHOUSE:
            return (Scene){ ato2LanhouseDialogues, GetAto2LanhouseCount(), SCENE_ATO2_IA_ESCREVE };

        case SCENE_ATO2_LANHOUSE_SEM:
            return (Scene){ ato2LanhouseSemDialogues, GetAto2LanhouseSemCount(), SCENE_ATO2_IA_ESCREVE };

        case SCENE_ATO2_IA_ESCREVE:
            return (Scene){ ato2IAEscreveDialogues, GetAto2IAEscreveCount(), SCENE_ATO2_C4_ZERO };

        case SCENE_ATO2_UMA_IDEIA:
            return (Scene){ ato2UmaIdeiaDialogues, GetAto2UmaIdeiaCount(), SCENE_ATO2_C4 };

        case SCENE_ATO2_COM_JOAO:
            return (Scene){ ato2ComJoaoDialogues, GetAto2ComJoaoCount(), SCENE_ATO2_C4 };

        case SCENE_ATO2_C4:
            return (Scene){ ato2C4Dialogues, GetAto2C4Count(), SCENE_ATO2_CG_ESCREVEM };

        case SCENE_ATO2_CG_ESCREVEM:
            return (Scene){ ato2CgEscrevemDialogues, GetAto2CgEscrevemCount(), SCENE_ATO2_CG_CASA_NOITE };

        case SCENE_ATO2_CG_CASA_NOITE:
            return (Scene){ ato2CgCasaNoiteDialogues, GetAto2CgCasaNoiteCount(), SCENE_ATO2_CG_FRENTE_ESCOLA };

        case SCENE_ATO2_CG_FRENTE_ESCOLA:
            return (Scene){ ato2CgFrenteEscolaDialogues, GetAto2CgFrenteEscolaCount(), SCENE_ATO2_CG_PROFESSORA };

        case SCENE_ATO2_CG_PROFESSORA: 
        return (Scene){ ato2CgProfessoraDialogues, GetAto2CgProfessoraCount(), SCENE_ATO3_ABERTURA };

        case SCENE_ATO2_C4_ZERO:
            return (Scene){ ato2C4ZeroDialogues, GetAto2C4ZeroCount(), SCENE_ATO2_CG_ENSAIO };

        case SCENE_ATO2_CG_ENSAIO:
            return (Scene){ ato2CgEnsaioDialogues, GetAto2CgEnsaioCount(), SCENE_ATO2_CG_FRENTE_ESCOLA };

        // Barra em 0 no fim do Ato 2: o Ato 3 comeca com o token de reserva
        // (roteiro do Ato 2 e do Ato 3). Antes ia direto para o epilogo.
        case SCENE_ATO2_CG_PROFESSORA_SEM_IA:
            return (Scene){ ato2CgProfessoraSemIADialogues, GetAto2CgProfessoraSemIACount(), SCENE_ATO3_ABERTURA };

        // ---------------- Ato 3 ----------------
        // A escolha, o desafio e os finais estao no game.c (FimDaCena).

        case SCENE_ATO3_ABERTURA:
            return (Scene){ ato3AberturaDialogues, GetAto3AberturaCount(), SCENE_ATO3_OFERTA };

        case SCENE_ATO3_RESERVA:
            return (Scene){ ato3ReservaDialogues, GetAto3ReservaCount(), SCENE_ATO3_OFERTA };

        case SCENE_ATO3_OFERTA:
            return (Scene){ ato3OfertaDialogues, GetAto3OfertaCount(), SCENE_ATO3_OFERTA };

        case SCENE_ATO3_IA_LE:
            return (Scene){ ato3IaLeDialogues, GetAto3IaLeCount(), SCENE_PRESENT };

        case SCENE_ATO3_IA_LE_IA_ESCREVEU:
            return (Scene){ ato3IaLeIaEscreveuDialogues, GetAto3IaLeIaEscreveuCount(), SCENE_PRESENT };

        case SCENE_ATO3_AJUDA:
            return (Scene){ ato3AjudaDialogues, GetAto3AjudaCount(), SCENE_ATO3_CONCLUSAO };

        case SCENE_ATO3_SOZINHA:
            return (Scene){ ato3SozinhaDialogues, GetAto3SozinhaCount(), SCENE_ATO3_CONCLUSAO };

        case SCENE_ATO3_CONCLUSAO:
            return (Scene){ ato3ConclusaoDialogues, GetAto3ConclusaoCount(), SCENE_ATO3_ETAPA_CONCLUIDA };

        case SCENE_ATO3_CONCLUSAO_IA_ESCREVEU:
            return (Scene){ ato3ConclusaoIaEscreveuDialogues, GetAto3ConclusaoIaEscreveuCount(), SCENE_ATO3_ETAPA_CONCLUIDA };

        case SCENE_ATO3_ETAPA_CONCLUIDA:
            return (Scene){ ato3EtapaConcluidaDialogues, GetAto3EtapaConcluidaCount(), SCENE_EPILOGUE };

        // ---------------- Epilogo ----------------

        case SCENE_EPILOGUE:
            return (Scene){ epilogueDialogues, GetEpilogueDialogueCount(), SCENE_PRESENT };


        default:
        {
            Scene empty = {0};

            return empty;
        }
    }
}
