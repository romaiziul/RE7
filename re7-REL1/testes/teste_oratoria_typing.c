#include <stdio.h>
#include <string.h>
#include <math.h>
#include "oratoria_typing.h"
#include "tokens.h"

static int checks, failures;
#define CHECK(x) do { checks++; if (!(x)) { failures++; printf("FALHOU linha %d: %s\n", __LINE__, #x); } } while (0)
static Oratoria jogo;

static void iniciar(Ato3Ajuda modo) {
    Oratoria_Iniciar(&jogo, modo, 42);
    Oratoria_Atualizar(&jogo, 0.3f);
}
static void digitar(const char *texto) {
    while (*texto) Oratoria_Digitar(&jogo, (unsigned char)*texto++);
}
static void proxima(void) { Oratoria_Atualizar(&jogo, 1.0f); Oratoria_Atualizar(&jogo, 0.3f); }

int main(void) {
    Tokens_Restaurar(2, false);
    iniciar(ATO3_AJUDA_SOZINHA);
    CHECK(!Oratoria_Selecionar(&jogo, -1));
    CHECK(!Oratoria_Selecionar(&jogo, 3));
    CHECK(Oratoria_Selecionar(&jogo, 1));
    CHECK(jogo.digitando && jogo.tamanho == 7 && jogo.progresso == 0);
    CHECK(!Oratoria_Selecionar(&jogo, 0));
    CHECK(jogo.limite >= 7 && jogo.limite <= 10);
    bool diferente = false; int contagem[256] = {0};
    for(int i=0;i<jogo.tamanho;i++) {
        diferente |= jogo.ordem[i] != i;
        CHECK(jogo.ordem[i] >= 0 && jogo.ordem[i] < jogo.tamanho);
        contagem[jogo.ordem[i]]++;
    }
    CHECK(diferente);
    for(int i=0;i<7;i++) CHECK(contagem[i] == 1);
    CHECK(!jogo.discurso.nitida[0]);
    CHECK(Oratoria_Digitar(&jogo, 'P') == ORATORIA_LETRA_CERTA);
    CHECK(jogo.progresso == 1);
    float antes=jogo.restante;
    CHECK(Oratoria_Digitar(&jogo, 'x') == ORATORIA_LETRA_ERRADA);
    CHECK(jogo.progresso == 1 && jogo.erros == 1);
    CHECK(fabsf(jogo.restante - (antes - 0.35f)) < 0.001f);
    CHECK(Oratoria_Digitar(&jogo, 0xFA) == ORATORIA_LETRA_CERTA);
    CHECK(Oratoria_Digitar(&jogo, 0x301) == ORATORIA_IGNORADA);
    digitar("BLICO");
    CHECK(!jogo.digitando && jogo.discurso.nitida[0]);
    CHECK(jogo.discurso.fase == EL_FASE_ACERTO);
    CHECK(Tokens_Valor() == 2);
    CHECK(Oratoria_Digitar(&jogo, 'q') == ORATORIA_IGNORADA);
    proxima();
    CHECK(jogo.discurso.etapa == 1);
    CHECK(Oratoria_Selecionar(&jogo, 0));
    digitar("amigos"); proxima();
    CHECK(jogo.discurso.etapa == 2);
    CHECK(Oratoria_Selecionar(&jogo, 2));
    digitar("confiante"); proxima();
    CHECK(jogo.discurso.fase == EL_FASE_CONCLUSAO);
    Oratoria_Atualizar(&jogo, 1.3f);
    CHECK(Oratoria_Avancar(&jogo));
    CHECK(EntreLinhas_Terminou(&jogo.discurso));

    iniciar(ATO3_AJUDA_TIRAR_OPCAO);
    CHECK(!Oratoria_Selecionar(&jogo, 2));
    CHECK(Oratoria_Selecionar(&jogo, 1));
    Oratoria_Atualizar(&jogo, -5);
    CHECK(jogo.restante == jogo.limite);
    CHECK(!Oratoria_Avancar(&jogo));
    digitar("publico");
    CHECK(jogo.discurso.nitida[0]);

    iniciar(ATO3_AJUDA_SOZINHA);
    CHECK(Oratoria_Selecionar(&jogo, 0));
    digitar("coragem");
    CHECK(jogo.discurso.fase == EL_FASE_ERRO_LIA);
    CHECK(jogo.discurso.errou[0] && !jogo.discurso.nitida[0]);
    CHECK(!jogo.esgotou && Tokens_Valor() == 2);
    Oratoria_Atualizar(&jogo, 0.3f); Oratoria_Avancar(&jogo);
    CHECK(jogo.discurso.fase == EL_FASE_ERRO_AMIGO);
    Oratoria_Atualizar(&jogo, 0.3f); Oratoria_Avancar(&jogo);
    CHECK(jogo.discurso.etapa == 1);

    iniciar(ATO3_AJUDA_SOZINHA);
    CHECK(Oratoria_Selecionar(&jogo, 1));
    Oratoria_Atualizar(&jogo, jogo.limite);
    CHECK(jogo.esgotou && !jogo.digitando && jogo.restante == 0);
    CHECK(jogo.discurso.fase == EL_FASE_ERRO_LIA && Tokens_Valor() == 2);
    CHECK(Oratoria_Digitar(&jogo, 'p') == ORATORIA_IGNORADA);
    Oratoria_Atualizar(&jogo, 0.3f); Oratoria_Avancar(&jogo);
    Oratoria_Atualizar(&jogo, 0.3f); Oratoria_Avancar(&jogo);
    CHECK(!jogo.esgotou && jogo.discurso.etapa == 1);

    iniciar(ATO3_AJUDA_SOZINHA);
    Oratoria_Selecionar(&jogo, 1);
    jogo.restante = 0.1f;
    Oratoria_Digitar(&jogo, 'z');
    CHECK(jogo.esgotou && jogo.restante == 0);

    iniciar(ATO3_AJUDA_SOZINHA);
    Oratoria_Atualizar(&jogo, 100);
    CHECK(!jogo.digitando && jogo.discurso.fase == EL_FASE_ESCOLHENDO);
    Oratoria_Selecionar(&jogo, 1); digitar("publico"); proxima();
    Oratoria_Selecionar(&jogo, 1); // familiares: letras repetidas
    CHECK(jogo.tamanho == 10);
    digitar("familiares");
    CHECK(jogo.progresso == 10 && jogo.discurso.fase == EL_FASE_ERRO_LIA);

    for(unsigned seed=0;seed<100;seed++) {
        Oratoria_Iniciar(&jogo, ATO3_AJUDA_SOZINHA, seed);
        Oratoria_Atualizar(&jogo, 0.3f); Oratoria_Selecionar(&jogo, 1);
        bool mudou=false;
        for(int i=0;i<7;i++) mudou |= jogo.letras[jogo.ordem[i]] != jogo.letras[i];
        CHECK(mudou);
        digitar("publico"); CHECK(jogo.discurso.nitida[0]);
    }
    printf("%d verificacoes, %d falhas\n",checks,failures);
    return failures ? 1 : 0;
}
