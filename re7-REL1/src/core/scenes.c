#include "scenes.h"
#include "story.h"
#include "story_ato1.h"
#include "story_ato2.h"
#include "story_ato3.h"


// =====================================================
// RETORNAR CENA
//
// "nextScene" e a cena seguinte quando nao ha escolha nem minijogo
// no fim da cena. Os casos com escolha ou minijogo estao no game.c
// (funcao FimDaCena).
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
            return (Scene){ ato2CorredorDialogues, GetAto2CorredorCount(), SCENE_ATO2_POSTER };

        case SCENE_ATO2_POSTER:
            return (Scene){ ato2PosterDialogues, GetAto2PosterCount(), SCENE_ATO2_LANHOUSE };

        case SCENE_ATO2_LANHOUSE:
            return (Scene){ ato2LanhouseDialogues, GetAto2LanhouseCount(), SCENE_ATO2_IA_ESCREVE };

        case SCENE_ATO2_IA_ESCREVE:
            return (Scene){ ato2IAEscreveDialogues, GetAto2IAEscreveCount(), SCENE_ATO2_ENSAIO };

        case SCENE_ATO2_UMA_IDEIA:
            return (Scene){ ato2UmaIdeiaDialogues, GetAto2UmaIdeiaCount(), SCENE_ATO2_DISCURSO };

        case SCENE_ATO2_FECHAR:
            return (Scene){ ato2FecharJanelaDialogues, GetAto2FecharJanelaCount(), SCENE_ATO2_DISCURSO };

        case SCENE_ATO2_DISCURSO:
            return (Scene){ ato2DiscursoDialogues, GetAto2DiscursoCount(), SCENE_ATO2_CG_ESCREVEM };

        case SCENE_ATO2_CG_ESCREVEM:
            return (Scene){ ato2CgEscrevemDialogues, GetAto2CgEscrevemCount(), SCENE_ATO2_CG_PROFESSORA };

        case SCENE_ATO2_CG_PROFESSORA: 
        return (Scene){ ato2CgProfessoraDialogues, GetAto2CgProfessoraCount(), SCENE_ATO3_ABERTURA };

        case SCENE_ATO2_ENSAIO:
            return (Scene){ ato2EnsaioDialogues, GetAto2EnsaioCount(), SCENE_ATO2_CG_ENSAIO };

        case SCENE_ATO2_CG_ENSAIO:
            return (Scene){ ato2CgEnsaioDialogues, GetAto2CgEnsaioCount(), SCENE_ATO3_ABERTURA }; 
        
        case SCENE_ATO3_ABERTURA:
            return (Scene){ ato3AberturaDialogues, GetAto3AberturaCount(), SCENE_ATO3_OFERTA };

        case SCENE_ATO3_RESERVA:
            return (Scene){ ato3ReservaDialogues, GetAto3ReservaCount(), SCENE_ATO3_OFERTA };

        case SCENE_ATO3_OFERTA:
            return (Scene){ ato3OfertaDialogues, GetAto3OfertaCount(), SCENE_EPILOGUE };
        
          

        // ---------------- Epilogo (provisorio) ----------------

        case SCENE_EPILOGUE:
            return (Scene){ epilogueDialogues, GetEpilogueDialogueCount(), SCENE_PRESENT };


        default:
        {
            Scene empty = {0};

            return empty;
        }
    }
}
