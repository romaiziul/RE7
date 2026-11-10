// ato3_logica.c
// Regras do Ato 3 e maquina de estados do minijogo "Entre as Linhas".
// Os textos seguem docs/roteiro/ATO 3.docx.
#include "ato3_logica.h"
#include "tokens.h"

#include <stddef.h>


// =====================================================
// REGRAS DO ATO
// =====================================================

bool Ato3_EntrarNoAto(void)
{
    return Tokens_UsarReserva();
}

void Ato3_AplicarAjuda(Ato3Ajuda ajuda)
{
    switch (ajuda)
    {
        case ATO3_AJUDA_IA_LE:       Tokens_Zerar();     break;
        case ATO3_AJUDA_TIRAR_OPCAO: Tokens_Gastar();    break;
        case ATO3_AJUDA_SOZINHA:     Tokens_Recuperar(); break;
    }
}

bool Ato3_TemDesafio(Ato3Ajuda ajuda)
{
    return ajuda != ATO3_AJUDA_IA_LE;
}

bool Ato3_TerminaNoEpilogo(void)
{
    // O erro no desafio nao mexe na barra: so a escolha conta.
    return Tokens_Valor() >= 1;
}

SceneID Ato3_CenaAposAbertura(void)
{
    return Ato3_EntrarNoAto() ? SCENE_ATO3_RESERVA : SCENE_ATO3_OFERTA;
}

SceneID Ato3_CenaDaAjuda(Ato3Ajuda ajuda, bool iaEscreveuDiscurso)
{
    switch (ajuda)
    {
        case ATO3_AJUDA_IA_LE:
            return iaEscreveuDiscurso ? SCENE_ATO3_IA_LE_IA_ESCREVEU : SCENE_ATO3_IA_LE;

        case ATO3_AJUDA_TIRAR_OPCAO:
            return SCENE_ATO3_AJUDA;

        default:
            return SCENE_ATO3_SOZINHA;
    }
}

SceneID Ato3_CenaDeConclusao(bool iaEscreveuDiscurso)
{
    return iaEscreveuDiscurso ? SCENE_ATO3_CONCLUSAO_IA_ESCREVEU : SCENE_ATO3_CONCLUSAO;
}


// =====================================================
// ETAPAS (roteiro do Ato 3, cenarios 2 e 3)
// =====================================================

// Cenario 2: a IA tirou uma opcao errada de cada palavra (2 alternativas).
static const Ato3Etapa ETAPAS_COM_AJUDA[ATO3_ETAPAS] =
{
    {
        "público", { "Coragem", "Público", NULL }, 2, 1, "Vaidade",
        "(gaguejando) C-coragem... não, espera...",
        "(sussurrando da plateia) Público, LIA! Público!"
    },
    {
        "amigos", { "Amigos", "Familiares", NULL }, 2, 0, "Pets",
        "(gaguejando) F-familiares... não, isso não...",
        "(sussurrando da plateia) Amigos, LIA!"
    },
    {
        "confiante", { "Amedrontada", "Confiante", NULL }, 2, 1, "Saudosa",
        "(gaguejando) A-amedrontada... não, isso não...",
        "(sussurrando da plateia) Confiante, LIA! Você consegue!"
    }
};

// Cenario 3: a Lia tenta sozinha (3 alternativas).
static const Ato3Etapa ETAPAS_SOZINHA[ATO3_ETAPAS] =
{
    {
        "público", { "Coragem", "Público", "Vaidade" }, 3, 1, NULL,
        "(gaguejando) Falo e-em... não, espera...",
        "(sussurrando da plateia) Público, LIA! Público!"
    },
    {
        "amigos", { "Amigos", "Familiares", "Pets" }, 3, 0, NULL,
        "(gaguejando) Ajuda d-de me-eus... não, isso não...",
        "(sussurrando da plateia) Amigos, LIA!"
    },
    {
        "confiante", { "Saudosa", "Amedrontada", "Confiante" }, 3, 2, NULL,
        "(gaguejando) B-bem mais... não, isso não...",
        "(sussurrando da plateia) Confiante, LIA! Você consegue!"
    }
};

const Ato3Etapa *Ato3_Etapa(Ato3Ajuda modo, int etapa)
{
    if (etapa < 0) etapa = 0;
    if (etapa >= ATO3_ETAPAS) etapa = ATO3_ETAPAS - 1;

    return (modo == ATO3_AJUDA_SOZINHA) ? &ETAPAS_SOZINHA[etapa]
                                        : &ETAPAS_COM_AJUDA[etapa];
}


// =====================================================
// MAQUINA DE ESTADOS DO MINIJOGO
// =====================================================

static void MudarFase(EntreLinhas *el, EntreLinhasFase fase)
{
    el->fase = fase;
    el->tempoFase = 0.0f;
    el->bloqueio = EL_TEMPO_BLOQUEIO;
}

// Depois de recuperar a palavra (certa ou com o amigo): proxima etapa ou conclusao
static void ProximaEtapa(EntreLinhas *el)
{
    el->etapa++;
    el->opcaoErrada = -1;

    if (el->etapa >= ATO3_ETAPAS)
        MudarFase(el, EL_FASE_CONCLUSAO);
    else
        MudarFase(el, EL_FASE_ESCOLHENDO);
}

void EntreLinhas_Iniciar(EntreLinhas *el, Ato3Ajuda modo)
{
    // "IA le o papel" nao tem desafio; por seguranca vira o modo com ajuda.
    if (modo == ATO3_AJUDA_IA_LE) modo = ATO3_AJUDA_TIRAR_OPCAO;

    el->modo = modo;
    el->etapa = 0;
    el->opcaoErrada = -1;

    for (int i = 0; i < ATO3_ETAPAS; i++)
    {
        el->nitida[i] = false;
        el->errou[i] = false;
    }

    MudarFase(el, EL_FASE_ESCOLHENDO);
}

EntreLinhasResposta EntreLinhas_Responder(EntreLinhas *el, int opcao)
{
    if (el->fase != EL_FASE_ESCOLHENDO || el->bloqueio > 0.0f)
        return EL_RESPOSTA_IGNORADA;

    const Ato3Etapa *etapa = Ato3_Etapa(el->modo, el->etapa);

    if (opcao < 0 || opcao >= etapa->total)
        return EL_RESPOSTA_IGNORADA;

    if (opcao == etapa->correta)
    {
        el->nitida[el->etapa] = true;
        MudarFase(el, EL_FASE_ACERTO);
        return EL_RESPOSTA_CERTA;
    }

    // Erro: nao repete a pergunta. A Lia gagueja, o amigo sopra e segue.
    el->opcaoErrada = opcao;
    el->errou[el->etapa] = true;
    MudarFase(el, EL_FASE_ERRO_LIA);
    return EL_RESPOSTA_ERRADA;
}

bool EntreLinhas_Avancar(EntreLinhas *el)
{
    if (el->bloqueio > 0.0f) return false;

    switch (el->fase)
    {
        case EL_FASE_ERRO_LIA:
            // o amigo sopra: a palavra certa aparece
            el->nitida[el->etapa] = true;
            MudarFase(el, EL_FASE_ERRO_AMIGO);
            return true;

        case EL_FASE_ERRO_AMIGO:
            ProximaEtapa(el);
            return true;

        case EL_FASE_CONCLUSAO:
            if (el->tempoFase < EL_TEMPO_CONCLUSAO) return false;
            MudarFase(el, EL_FASE_FIM);
            return true;

        default:
            return false;
    }
}

void EntreLinhas_Atualizar(EntreLinhas *el, float dt)
{
    if (dt < 0.0f) dt = 0.0f;

    el->tempoFase += dt;

    if (el->bloqueio > 0.0f)
    {
        el->bloqueio -= dt;
        if (el->bloqueio < 0.0f) el->bloqueio = 0.0f;
    }

    // o acerto avanca sozinho depois da animacao
    if (el->fase == EL_FASE_ACERTO && el->tempoFase >= EL_TEMPO_ACERTO)
        ProximaEtapa(el);
}

bool EntreLinhas_Terminou(const EntreLinhas *el)
{
    return el->fase == EL_FASE_FIM;
}

int EntreLinhas_Recuperadas(const EntreLinhas *el)
{
    int total = 0;

    for (int i = 0; i < ATO3_ETAPAS; i++)
        if (el->nitida[i]) total++;

    return total;
}
