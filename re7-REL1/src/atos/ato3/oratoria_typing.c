#include "oratoria_typing.h"
#include <string.h>
#include <math.h>

static int Minuscula(int c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 32;
    return c;
}

static int SemAcento(int c) {
    c = Minuscula(c);
    if (c >= 0xE0 && c <= 0xE5) return 'a';
    if (c >= 0xE8 && c <= 0xEB) return 'e';
    if (c >= 0xEC && c <= 0xEF) return 'i';
    if (c >= 0xF2 && c <= 0xF6) return 'o';
    if (c >= 0xF9 && c <= 0xFC) return 'u';
    if (c == 0xE7) return 'c';
    return c;
}

// O roteiro legado tem versoes Latin-1 e UTF-8. Aceita ambas sem separar
// os bytes de um acento em duas letras. Novos textos devem usar UTF-8.
static int LerLetra(const unsigned char **p) {
    int c = *(*p)++;
    if (c >= 0xC2 && c <= 0xDF && **p >= 0x80 && **p <= 0xBF) {
        c = ((c & 31) << 6) | (*(*p)++ & 63);
    }
    return Minuscula(c);
}

static unsigned Sortear(Oratoria *jogo) {
    jogo->aleatorio = jogo->aleatorio * 1664525u + 1013904223u;
    return jogo->aleatorio;
}

static void Encerrar(Oratoria *jogo, bool esgotou) {
    const Ato3Etapa *etapa = Ato3_Etapa(jogo->discurso.modo, jogo->discurso.etapa);
    jogo->digitando = false;
    jogo->esgotou = esgotou;
    int resposta = jogo->opcao;
    if (esgotou) {
        jogo->restante = 0;
        // Usa o caminho narrativo de erro, mesmo se escolheu a palavra certa.
        resposta = (etapa->correta + 1) % etapa->total;
    }
    EntreLinhas_Responder(&jogo->discurso, resposta);
}

void Oratoria_Iniciar(Oratoria *jogo, Ato3Ajuda modo, unsigned semente) {
    memset(jogo, 0, sizeof(*jogo));
    jogo->aleatorio = semente;
    jogo->opcao = -1;
    EntreLinhas_Iniciar(&jogo->discurso, modo);
}

bool Oratoria_Selecionar(Oratoria *jogo, int opcao) {
    if (jogo->digitando || jogo->discurso.fase != EL_FASE_ESCOLHENDO ||
        jogo->discurso.bloqueio > 0) return false;
    const Ato3Etapa *etapa = Ato3_Etapa(jogo->discurso.modo, jogo->discurso.etapa);
    if (opcao < 0 || opcao >= etapa->total) return false;

    const unsigned char *texto = (const unsigned char *)etapa->opcoes[opcao];
    jogo->tamanho = 0;
    while (*texto && jogo->tamanho < ORATORIA_MAX_LETRAS) {
        int i = jogo->tamanho++;
        jogo->letras[i] = LerLetra(&texto);
        jogo->ordem[i] = i;
    }
    if (*texto || jogo->tamanho == 0) return false;
    for (int i = jogo->tamanho - 1; i > 0; i--) {
        int j = (int)(Sortear(jogo) % (unsigned)(i + 1));
        int tmp = jogo->ordem[i]; jogo->ordem[i] = jogo->ordem[j]; jogo->ordem[j] = tmp;
    }
    bool igual = true;
    for (int i = 0; i < jogo->tamanho; i++)
        if (jogo->letras[jogo->ordem[i]] != jogo->letras[i]) igual = false;
    if (igual) {
        for (int i = 1; i < jogo->tamanho; i++) {
            if (jogo->letras[i] != jogo->letras[0]) {
                int tmp = jogo->ordem[0]; jogo->ordem[0] = jogo->ordem[i]; jogo->ordem[i] = tmp;
                break;
            }
        }
    }
    jogo->opcao = opcao;
    jogo->progresso = jogo->erros = 0;
    jogo->esgotou = false;
    jogo->limite = 2.5f + jogo->tamanho * 0.7f;
    if (jogo->limite < ORATORIA_TEMPO_MINIMO) jogo->limite = ORATORIA_TEMPO_MINIMO;
    if (jogo->limite > ORATORIA_TEMPO_MAXIMO) jogo->limite = ORATORIA_TEMPO_MAXIMO;
    jogo->restante = jogo->limite;
    jogo->digitando = true;
    return true;
}

OratoriaEntrada Oratoria_Digitar(Oratoria *jogo, int codepoint) {
    if (!jogo->digitando) return ORATORIA_IGNORADA;
    int c = SemAcento(codepoint);
    // Ignora teclas de controle e marcas combinantes (acentos em NFD).
    if (c < 'a' || c > 'z') return ORATORIA_IGNORADA;
    if (c != SemAcento(jogo->letras[jogo->progresso])) {
        jogo->erros++;
        jogo->restante -= ORATORIA_PENALIDADE;
        if (jogo->restante <= 0) Encerrar(jogo, true);
        return ORATORIA_LETRA_ERRADA;
    }
    jogo->progresso++;
    if (jogo->progresso < jogo->tamanho) return ORATORIA_LETRA_CERTA;
    Encerrar(jogo, false);
    return jogo->discurso.fase == EL_FASE_ACERTO ? ORATORIA_PALAVRA_CERTA : ORATORIA_PALAVRA_ERRADA;
}

void Oratoria_Atualizar(Oratoria *jogo, float dt) {
    if (!isfinite(dt) || dt < 0) dt = 0;
    if (jogo->digitando) {
        jogo->restante -= dt;
        if (jogo->restante <= 0) Encerrar(jogo, true);
        return;
    }
    int anterior = jogo->discurso.etapa;
    EntreLinhas_Atualizar(&jogo->discurso, dt);
    if (jogo->discurso.etapa != anterior) jogo->esgotou = false;
}

bool Oratoria_Avancar(Oratoria *jogo) {
    if (jogo->digitando) return false;
    int anterior = jogo->discurso.etapa;
    bool avancou = EntreLinhas_Avancar(&jogo->discurso);
    if (jogo->discurso.etapa != anterior) jogo->esgotou = false;
    return avancou;
}
