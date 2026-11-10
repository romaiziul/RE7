// teste_ato3.c
// Teste automatico das regras do Ato 3 e do desafio "Entre as Linhas".
// Nao abre janela: testa so ato3_logica.c e tokens.c (sem raylib).
//
// Compilar e rodar (Windows, w64devkit), dentro de re7-REL1:
//   gcc testes/teste_ato3.c src/atos/ato3/ato3_logica.c src/systems/tokens.c -Isrc/core -Isrc/systems -Isrc/atos/ato3 -o teste_ato3.exe
//   .\teste_ato3.exe
// Compilar (macOS/Linux), dentro de re7-REL1:
//   cc testes/teste_ato3.c src/atos/ato3/ato3_logica.c src/systems/tokens.c -Isrc/core -Isrc/systems -Isrc/atos/ato3 -o teste_ato3 && ./teste_ato3
//
// Sai com codigo 0 se tudo passou.

#include <stdio.h>
#include <string.h>

#include "ato3_logica.h"
#include "tokens.h"

static int testes = 0;
static int falhas = 0;

#define VERIFICA(condicao, descricao)                                   \
    do {                                                                \
        testes++;                                                       \
        if (condicao) printf("  ok    %s\n", descricao);                \
        else { falhas++; printf("  FALHA %s  (%s:%d)\n", descricao, __FILE__, __LINE__); } \
    } while (0)

typedef enum { RESULTADO_EPILOGO, RESULTADO_FINAL_RUIM } Resultado;

typedef struct
{
    Resultado resultado;
    bool reservaAtivou;
    int tokensDepoisDaAbertura;
    int tokensDepoisDaEscolha;
    int tokensNoFim;
    int erros;
} Partida;

// Joga o desafio inteiro. errar[i]: true escolhe uma opcao errada na palavra i.
static void JogarDesafio(EntreLinhas *el, const bool errar[ATO3_ETAPAS])
{
    for (int passos = 0; passos < 200 && !EntreLinhas_Terminou(el); passos++)
    {
        EntreLinhas_Atualizar(el, 0.3f);       // passa o bloqueio entre fases

        if (el->fase == EL_FASE_ESCOLHENDO)
        {
            const Ato3Etapa *e = Ato3_Etapa(el->modo, el->etapa);
            int opcao = errar[el->etapa] ? (e->correta + 1) % e->total : e->correta;
            EntreLinhas_Responder(el, opcao);
        }
        else
        {
            EntreLinhas_Avancar(el);
        }
    }
}

// Ato 3 completo: abertura (reserva), escolha, desafio e fim.
static Partida Jogar(int tokensAoEntrar, bool reservaJaUsada, Ato3Ajuda ajuda, bool errarTudo)
{
    Partida p = {0};

    Tokens_Restaurar(tokensAoEntrar, reservaJaUsada);

    p.reservaAtivou = (Ato3_CenaAposAbertura() == SCENE_ATO3_RESERVA);
    p.tokensDepoisDaAbertura = Tokens_Valor();

    Ato3_AplicarAjuda(ajuda);
    p.tokensDepoisDaEscolha = Tokens_Valor();

    if (!Ato3_TemDesafio(ajuda))
    {
        p.resultado = RESULTADO_FINAL_RUIM;
        p.tokensNoFim = Tokens_Valor();
        return p;
    }

    bool errar[ATO3_ETAPAS] = { errarTudo, errarTudo, errarTudo };
    EntreLinhas el;
    EntreLinhas_Iniciar(&el, ajuda);
    JogarDesafio(&el, errar);

    for (int i = 0; i < ATO3_ETAPAS; i++) if (el.errou[i]) p.erros++;

    p.tokensNoFim = Tokens_Valor();
    p.resultado = Ato3_TerminaNoEpilogo() ? RESULTADO_EPILOGO : RESULTADO_FINAL_RUIM;
    return p;
}

int main(void)
{
    Partida p;

    printf("1. Entrada com 0 tokens: reserva ativada e escolha disponivel\n");
    p = Jogar(0, false, ATO3_AJUDA_SOZINHA, false);
    VERIFICA(p.reservaAtivou, "a reserva entra");
    VERIFICA(p.tokensDepoisDaAbertura == 1, "a barra vai para 1 antes da escolha");
    VERIFICA(Tokens_ReservaUsada(), "a reserva fica marcada como usada");
    VERIFICA(p.resultado == RESULTADO_EPILOGO && p.tokensNoFim == 2, "reserva + sozinha: epilogo com 2 (tabela do roteiro)");
    p = Jogar(0, false, ATO3_AJUDA_TIRAR_OPCAO, false);
    VERIFICA(p.resultado == RESULTADO_FINAL_RUIM && p.tokensNoFim == 0, "reserva + ajuda: final ruim com 0 (tabela do roteiro)");

    printf("2. Entrada com 1 token + ajuda: final ruim\n");
    p = Jogar(1, false, ATO3_AJUDA_TIRAR_OPCAO, false);
    VERIFICA(!p.reservaAtivou, "com 1 token a reserva nao entra");
    VERIFICA(p.tokensDepoisDaEscolha == 0, "a ajuda custa 1 token");
    VERIFICA(p.resultado == RESULTADO_FINAL_RUIM, "final ruim");

    printf("3. Entrada com 1 token + sozinha: epilogo\n");
    p = Jogar(1, false, ATO3_AJUDA_SOZINHA, false);
    VERIFICA(p.tokensDepoisDaEscolha == 2 && p.resultado == RESULTADO_EPILOGO, "barra 2, epilogo");

    printf("4. Entrada com 2 tokens + ajuda: epilogo\n");
    p = Jogar(2, false, ATO3_AJUDA_TIRAR_OPCAO, false);
    VERIFICA(p.tokensNoFim == 1 && p.resultado == RESULTADO_EPILOGO, "barra 1, epilogo");

    printf("5. Entrada com 2 tokens + sozinha: epilogo\n");
    p = Jogar(2, false, ATO3_AJUDA_SOZINHA, false);
    VERIFICA(p.tokensNoFim == 3 && p.resultado == RESULTADO_EPILOGO, "barra 3, epilogo");

    printf("6. Entrada com 3 tokens + ajuda ou sozinha: epilogo\n");
    p = Jogar(3, false, ATO3_AJUDA_TIRAR_OPCAO, false);
    VERIFICA(p.tokensNoFim == 2 && p.resultado == RESULTADO_EPILOGO, "ajuda: barra 2, epilogo");
    p = Jogar(3, false, ATO3_AJUDA_SOZINHA, true);
    VERIFICA(p.tokensNoFim == 3 && p.resultado == RESULTADO_EPILOGO, "sozinha (errando tudo): barra 3, epilogo");

    printf("7. IA le o papel: final ruim com qualquer barra\n");
    for (int t = 0; t <= TOKENS_MAX; t++)
    {
        p = Jogar(t, false, ATO3_AJUDA_IA_LE, false);
        char msg[80];
        snprintf(msg, sizeof(msg), "entrada com %d: barra zerada e final ruim", t);
        VERIFICA(p.tokensNoFim == 0 && p.resultado == RESULTADO_FINAL_RUIM, msg);
    }
    VERIFICA(!Ato3_TemDesafio(ATO3_AJUDA_IA_LE), "sem desafio (vai direto ao final ruim)");

    printf("8. Erros nas tres palavras: o desafio termina\n");
    {
        EntreLinhas el;
        bool errar[ATO3_ETAPAS] = { true, true, true };
        Tokens_Restaurar(2, false);
        EntreLinhas_Iniciar(&el, ATO3_AJUDA_SOZINHA);
        JogarDesafio(&el, errar);
        VERIFICA(EntreLinhas_Terminou(&el), "chega ao fim");
        VERIFICA(EntreLinhas_Recuperadas(&el) == 3, "as tres palavras ficam nitidas");
        VERIFICA(el.errou[0] && el.errou[1] && el.errou[2], "as tres foram sopradas pelo amigo");
        VERIFICA(Tokens_Valor() == 2, "errar nao mexe na barra");
    }

    printf("9. Acertos nas tres palavras: conclusao normal\n");
    {
        EntreLinhas el;
        bool errar[ATO3_ETAPAS] = { false, false, false };
        EntreLinhas_Iniciar(&el, ATO3_AJUDA_TIRAR_OPCAO);
        JogarDesafio(&el, errar);
        VERIFICA(EntreLinhas_Terminou(&el) && EntreLinhas_Recuperadas(&el) == 3, "termina com as tres palavras");
        VERIFICA(!el.errou[0] && !el.errou[1] && !el.errou[2], "nenhuma soprada");
    }

    printf("10. Limite maximo de 3 tokens\n");
    Tokens_Restaurar(3, false);
    Ato3_AplicarAjuda(ATO3_AJUDA_SOZINHA);
    VERIFICA(Tokens_Valor() == 3, "sozinha com 3 continua em 3");
    Tokens_Restaurar(9, false);
    VERIFICA(Tokens_Valor() == 3, "restaurar acima de 3 limita em 3");

    printf("11. A reserva so acontece uma vez\n");
    Tokens_Restaurar(0, false);
    VERIFICA(Ato3_CenaAposAbertura() == SCENE_ATO3_RESERVA, "primeira vez: reserva");
    Tokens_Restaurar(0, Tokens_ReservaUsada());
    VERIFICA(Ato3_CenaAposAbertura() == SCENE_ATO3_OFERTA, "segunda vez: segue sem reserva");
    VERIFICA(Tokens_Valor() == 0, "a barra continua em 0");
    Tokens_Reiniciar();
    VERIFICA(!Tokens_ReservaUsada() && Tokens_Valor() == 3, "novo jogo: reserva disponivel de novo e barra em 3");

    printf("12. Fala diferente se a IA escreveu o discurso no Ato 2\n");
    VERIFICA(Ato3_CenaDeConclusao(true) == SCENE_ATO3_CONCLUSAO_IA_ESCREVEU, "conclusao: fala 'Nao fui eu que escrevi'");
    VERIFICA(Ato3_CenaDeConclusao(false) == SCENE_ATO3_CONCLUSAO, "conclusao: fala 'Foi isso que a gente escreveu junto'");
    VERIFICA(Ato3_CenaDaAjuda(ATO3_AJUDA_IA_LE, true) == SCENE_ATO3_IA_LE_IA_ESCREVEU, "IA le: fala 'Eu nem sei o que esta escrito aqui'");
    VERIFICA(Ato3_CenaDaAjuda(ATO3_AJUDA_IA_LE, false) == SCENE_ATO3_IA_LE, "IA le: fala 'Eu nem li o que a gente escreveu'");
    VERIFICA(Ato3_CenaDaAjuda(ATO3_AJUDA_TIRAR_OPCAO, true) == SCENE_ATO3_AJUDA, "ajuda leva ao desafio com ajuda");
    VERIFICA(Ato3_CenaDaAjuda(ATO3_AJUDA_SOZINHA, true) == SCENE_ATO3_SOZINHA, "sozinha leva ao desafio sozinha");

    printf("Extras: dados das etapas e cliques repetidos\n");
    {
        const char *certas[ATO3_ETAPAS] = { "Público", "Amigos", "Confiante" };

        for (int i = 0; i < ATO3_ETAPAS; i++)
        {
            const Ato3Etapa *a = Ato3_Etapa(ATO3_AJUDA_TIRAR_OPCAO, i);
            const Ato3Etapa *s = Ato3_Etapa(ATO3_AJUDA_SOZINHA, i);
            char msg[96];

            snprintf(msg, sizeof(msg), "palavra %d: 2 opcoes com ajuda, 3 sozinha, resposta '%s'", i + 1, certas[i]);
            VERIFICA(a->total == 2 && s->total == 3 &&
                     strcmp(a->opcoes[a->correta], certas[i]) == 0 &&
                     strcmp(s->opcoes[s->correta], certas[i]) == 0, msg);

            // a opcao que a IA tira e uma das erradas do modo sozinha
            bool removidaErrada = false;
            for (int k = 0; k < s->total; k++)
                if (k != s->correta && strcmp(s->opcoes[k], a->removida) == 0) removidaErrada = true;

            snprintf(msg, sizeof(msg), "palavra %d: a IA tira uma opcao errada ('%s')", i + 1, a->removida);
            VERIFICA(removidaErrada, msg);
        }

        EntreLinhas el;
        EntreLinhas_Iniciar(&el, ATO3_AJUDA_TIRAR_OPCAO);
        VERIFICA(EntreLinhas_Responder(&el, 1) == EL_RESPOSTA_IGNORADA, "clique logo ao abrir e ignorado (bloqueio)");
        EntreLinhas_Atualizar(&el, 0.3f);
        VERIFICA(EntreLinhas_Responder(&el, 2) == EL_RESPOSTA_IGNORADA, "opcao que nao existe com ajuda e ignorada");
        VERIFICA(EntreLinhas_Responder(&el, 1) == EL_RESPOSTA_CERTA, "primeiro clique responde");
        VERIFICA(EntreLinhas_Responder(&el, 1) == EL_RESPOSTA_IGNORADA, "segundo clique no mesmo instante e ignorado");
        EntreLinhas_Atualizar(&el, 0.3f);
        VERIFICA(EntreLinhas_Responder(&el, 0) == EL_RESPOSTA_IGNORADA, "durante a animacao do acerto nao responde");
        VERIFICA(el.etapa == 0, "nao avancou duas etapas");
        EntreLinhas_Atualizar(&el, EL_TEMPO_ACERTO);
        VERIFICA(el.etapa == 1 && el.fase == EL_FASE_ESCOLHENDO, "depois da animacao vai para a palavra 2");
    }

    printf("\n%d testes, %d falhas\n", testes, falhas);
    return falhas == 0 ? 0 : 1;
}
