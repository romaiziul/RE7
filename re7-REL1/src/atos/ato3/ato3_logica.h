// ato3_logica.h
// Regras do Ato 3 e estado do minijogo "Entre as Linhas", sem desenho.
// Nao depende da raylib: so de tokens.h. Assim as regras podem ser testadas
// sozinhas (testes/teste_ato3.c). O desenho fica em entre_linhas.c.
#ifndef ATO3_LOGICA_H
#define ATO3_LOGICA_H

#include <stdbool.h>

#include "scenes.h"     // so os numeros das cenas (sem raylib)

#define ATO3_ETAPAS      3      // tres palavras borradas
#define ATO3_MAX_OPCOES  3


// =====================================================
// ESCOLHA DO ATO 3: o que pedir a IA sobre o papel
// =====================================================

typedef enum
{
    ATO3_AJUDA_IA_LE = 0,       // a IA le o papel inteiro: zera a barra, final ruim
    ATO3_AJUDA_TIRAR_OPCAO,     // a IA tira uma opcao errada por palavra: -1
    ATO3_AJUDA_SOZINHA          // a Lia tenta sozinha: +1 (maximo 3)
} Ato3Ajuda;

// Entrada no ato: se a barra estiver em 0 e a reserva ainda nao foi usada,
// a IA volta com 1 token. Devolve true se a reserva entrou agora.
bool Ato3_EntrarNoAto(void);

// Aplica na barra o efeito da escolha.
void Ato3_AplicarAjuda(Ato3Ajuda ajuda);

// true se a escolha leva ao desafio (todas, menos "IA le o papel").
bool Ato3_TemDesafio(Ato3Ajuda ajuda);

// Depois do desafio: com pelo menos 1 token, epilogo; com 0, final ruim.
bool Ato3_TerminaNoEpilogo(void);

// Proxima cena depois da abertura. Usa a reserva se a barra estiver em 0
// (SCENE_ATO3_RESERVA); senao, SCENE_ATO3_OFERTA.
SceneID Ato3_CenaAposAbertura(void);

// Cena que segue a escolha (a fala muda se a IA escreveu o discurso no Ato 2).
SceneID Ato3_CenaDaAjuda(Ato3Ajuda ajuda, bool iaEscreveuDiscurso);

// Cena depois do desafio (a fala da Lia muda se a IA escreveu o discurso).
SceneID Ato3_CenaDeConclusao(bool iaEscreveuDiscurso);


// =====================================================
// MINIJOGO "ENTRE AS LINHAS"
// =====================================================

typedef struct
{
    const char *palavra;                    // como aparece no discurso ("público")
    const char *opcoes[ATO3_MAX_OPCOES];    // como aparecem nos botoes
    int total;                              // 2 (com ajuda) ou 3 (sozinha)
    int correta;                            // indice da opcao certa
    const char *removida;                   // com ajuda: a opcao que a IA tirou (NULL sozinha)
    const char *falaErroLia;                // a Lia gagueja
    const char *falaAmigo;                  // o amigo sopra da plateia
} Ato3Etapa;

// Dados da etapa (0 a 2) no modo escolhido. "modo" e ATO3_AJUDA_TIRAR_OPCAO
// ou ATO3_AJUDA_SOZINHA.
const Ato3Etapa *Ato3_Etapa(Ato3Ajuda modo, int etapa);

typedef enum
{
    EL_FASE_ESCOLHENDO,     // esperando a jogadora escolher a palavra
    EL_FASE_ACERTO,         // a palavra ganha nitidez; avanca sozinho
    EL_FASE_ERRO_LIA,       // a Lia gagueja (clique para continuar)
    EL_FASE_ERRO_AMIGO,     // o amigo sopra; a palavra fica nitida (clique)
    EL_FASE_CONCLUSAO,      // o papel inteiro fica nitido (clique)
    EL_FASE_FIM             // terminou: o jogo segue a historia
} EntreLinhasFase;

typedef enum
{
    EL_RESPOSTA_IGNORADA,   // fora da hora (animacao, clique repetido) ou opcao invalida
    EL_RESPOSTA_CERTA,
    EL_RESPOSTA_ERRADA
} EntreLinhasResposta;

typedef struct
{
    Ato3Ajuda modo;
    int etapa;                      // 0 a 2 (3 na conclusao)
    EntreLinhasFase fase;
    float tempoFase;                // segundos desde que a fase comecou
    float bloqueio;                 // entrada ignorada enquanto > 0 (evita clique duplo)
    int opcaoErrada;                // ultima opcao errada escolhida (-1 nenhuma)
    bool nitida[ATO3_ETAPAS];       // palavra ja recuperada
    bool errou[ATO3_ETAPAS];        // a palavra foi recuperada com a ajuda do amigo
} EntreLinhas;

// Tempos das animacoes (segundos)
#define EL_TEMPO_ACERTO      0.9f   // acerto: a palavra fica nitida e avanca sozinho
#define EL_TEMPO_BLOQUEIO    0.25f  // depois de cada mudanca de fase
#define EL_TEMPO_CONCLUSAO   1.2f   // conclusao: so aceita continuar depois disso

void EntreLinhas_Iniciar(EntreLinhas *el, Ato3Ajuda modo);

// Responde a etapa atual. So vale na fase ESCOLHENDO e fora do bloqueio.
EntreLinhasResposta EntreLinhas_Responder(EntreLinhas *el, int opcao);

// Clique/ESPACO nas fases que esperam a jogadora (erro e conclusao).
// Devolve true se a fase mudou.
bool EntreLinhas_Avancar(EntreLinhas *el);

// Passa o tempo (animacoes e bloqueio). Chamar uma vez por quadro.
void EntreLinhas_Atualizar(EntreLinhas *el, float dt);

bool EntreLinhas_Terminou(const EntreLinhas *el);

// Quantas palavras ja foram recuperadas (0 a 3).
int EntreLinhas_Recuperadas(const EntreLinhas *el);

#endif
