# 🎮 RE:7 - JOGO INTEGRADO

<p align="center">
  <img src="re7-REL1/assets/comum/logo.png" alt="Logo Re:7" width="250">
</p>

<p align="center">
  <strong>2007 ligou, você vai atender?</strong>
</p>

---

## 📑 Sumário

- [Sobre o Projeto](#sobre)
- [Sobre o Jogo](#jogo)
- [Como compilar e executar](#executar)
- [Como jogar](#jogar)
- [Modo desenvolvedor e testes](#dev)
- [Organização do Repositório](#organizacao)
- [Repositórios do projeto](#repositorios)
- [Gestão do Projeto](#gestao)
- [Equipe](#equipe)
- [Links Importantes](#links)

---

<a id="sobre"></a>
## 📖 Sobre o Projeto

**Re:7** é um projeto desenvolvido por estudantes do segundo período de **ADS**, da **CESAR School**, como parte de um Projeto Integrador que visa utilizar conhecimentos de todas as cadeiras simultaneamente.

Este repositório concentra a **versão integrada do jogo**, com os três atos, os minijogos, os sistemas compartilhados, os recursos gráficos e sonoros e os testes. A documentação acadêmica, os diagramas, os protótipos e as evidências das entregas ficam no repositório [projeto2-squad2](https://github.com/Vini-Lino/projeto2-squad2).

---

<a id="jogo"></a>
## 🎮 Sobre o Jogo

**Gênero:** Visual Novel / RPG de escolhas

**Tema:** Letramento em Inteligência Artificial, uso crítico da tecnologia e tomada de decisões.

**Ambientação:** 2007, em uma representação inspirada na cultura, tecnologia e estética digital dos anos 2000.

**Público-alvo:** Adolescentes e jovens adultos, incluindo tanto usuários familiarizados com ferramentas de IA quanto pessoas com pouco conhecimento sobre seu funcionamento, limites e riscos.

### 📜 Sinopse

Após uma interação inesperada com uma Inteligência Artificial, uma mulher adulta é transportada de volta para 2007, período em que ainda era adolescente.

Para encontrar uma forma de retornar ao presente, ela deverá enfrentar situações e desafios com a ajuda de uma IA que parece saber mais do que deveria.

### 🗺️ O que o jogo contém

- **Menu principal:** Novo Jogo, Continuar (usa o save), Créditos e Sair.
- **Ato 1:** exploração do quarto de 2007 e o minijogo da senha do computador, com lógica proposicional e oferta de ajuda da IA.
- **Ato 2:** corrida até a escola e o reencontro no corredor.
- **Ato 3:** o discurso no auditório e o desafio "Entre as Linhas", com escolha e digitação das palavras.
- **Epílogo:** desfecho influenciado pelas escolhas e tela de encerramento.
- **Barra de tokens da IA:** a ajuda da IA tem custo e pode acabar ao longo do jogo.

---

## ⚙️ Tecnologias

* **C**: linguagem utilizada para a lógica e as funcionalidades do jogo.
* **Raylib**: biblioteca utilizada para interface gráfica, imagens, textos, áudio e interações.
* **GitHub**: versionamento e colaboração no código.

---

<a id="executar"></a>
# 🛠️ Como compilar e executar

O código do jogo fica na pasta [`re7-REL1`](re7-REL1).

### Requisitos

- Windows com o compilador **GCC** (MinGW-w64, por exemplo o w64devkit)
- Biblioteca **raylib** em `C:/raylib/raylib/src`

### Compilar

Abra o PowerShell na pasta `re7-REL1` e rode:

```powershell
.\build.ps1
```

O script compila todos os arquivos de `src` e gera o `re7.exe`.

### Executar

Ainda dentro de `re7-REL1`:

```powershell
.\re7.exe
```

> O jogo precisa ser aberto a partir da pasta `re7-REL1`, porque lê as imagens, as fontes e os sons em `assets/`.

### Jogar sem instalar a raylib

Depois de compilado, o `re7.exe` não precisa da raylib instalada. Para jogar em outro computador, basta copiar o `re7.exe` junto com a pasta `assets`, mantendo a mesma estrutura.

---

<a id="jogar"></a>
# 🕹️ Como jogar

| Tecla | Ação |
|---|---|
| Mouse | Clicar nos botões, nos objetos do quarto e arrastar os caracteres da senha |
| ESPAÇO ou ENTER | Avançar as falas |
| ESC | Abrir o menu de pausa (o mesmo da engrenagem), de onde dá para voltar ao menu |
| F1 | Mostrar ou esconder a ajuda no quarto |
| F2 | Ligar ou desligar o som |
| Corrida (Ato 2) | ESPAÇO, W ou seta para cima para pular |
| Janela da IA | Clique, ou teclas 1 e 2 |
| Entre as Linhas (Ato 3) | Teclado para digitar as palavras, BACKSPACE apaga |

---

<a id="dev"></a>
# 🧪 Modo desenvolvedor e testes

O repositório tem dois modos de compilação:

- `.\build.ps1` gera o `re7.exe`, a versão normal do jogo.
- `.\build.ps1 -Dev` gera o `re7_dev.exe`, com um painel para navegar entre os atos e testar cenários (F10 abre o painel, F6 pula a fala, F7 avança a etapa, F8 reinicia o ato). Esse modo salva em `save_dev.dat`.

A pasta [`re7-REL1/testes`](re7-REL1/testes) tem programas de teste separados do jogo:

- `teste_senha.c`: abre só o minijogo da senha do Ato 1.
- `teste_ato3.c`: testa as regras do Ato 3 e do desafio "Entre as Linhas", sem abrir janela.
- `captura_telas.c`: gera imagens das telas do Ato 3 e do painel de desenvolvedor.

As instruções de compilação de cada teste estão no topo do próprio arquivo e no [README da pasta re7-REL1](re7-REL1/README.md).

---

<a id="organizacao"></a>
# 📁 Organização do Repositório

```text
📦 RE7
├── 📁 re7-REL1
│   ├── 📁 src
│   │   ├── main.c
│   │   ├── 📁 atos
│   │   │   ├── 📁 ato1      quarto e minijogo da senha
│   │   │   ├── 📁 ato2      corrida e reencontro na escola
│   │   │   └── 📁 ato3      discurso e desafio Entre as Linhas
│   │   ├── 📁 core          gerenciamento do jogo e das cenas
│   │   ├── 📁 story         textos e falas
│   │   ├── 📁 engine        interface, recursos gráficos e áudio
│   │   ├── 📁 systems       quarto, tokens e barra de energia da IA
│   │   ├── 📁 minigames     módulos dos minijogos
│   │   └── 📁 dev           modo desenvolvedor
│   │
│   ├── 📁 assets
│   │   ├── 📁 comum         interface, menu e personagens
│   │   ├── 📁 ato1  📁 ato2  📁 ato3  📁 epilogo
│   │   ├── 📁 fonts
│   │   ├── 📁 music         trilhas
│   │   └── 📁 sounds        efeitos sonoros
│   │
│   ├── 📁 docs
│   │   ├── 📁 roteiro       roteiros dos atos, epílogo e visão geral
│   │   └── documentacao_audio.md
│   │
│   ├── 📁 testes
│   ├── build.ps1
│   └── README.md
│
└── README.md
```

---

<a id="repositorios"></a>
# 🗂️ Repositórios do projeto

O Re:7 está organizado em dois repositórios complementares.

## 🎮 RE7 (este repositório)

Concentra a evolução e a integração do jogo completo. Para acompanhar o código da versão integrada e contribuir com o jogo, utilize este repositório.

🔗 **Endereço para clonagem:** https://github.com/romaiziul/RE7.git

## 📚 projeto2-squad2

🔗 **Repositório:** [projeto2-squad2](https://github.com/Vini-Lino/projeto2-squad2)

Preserva os registros acadêmicos do Projeto Integrador:

- épicos e histórias de usuário, com o status de cada uma;
- diagramas de atividades e protótipos Lo-Fi e Hi-Fi;
- registros de programação em pares e contribuições por sprint;
- núcleo do jogo desenvolvido para a entrega de PIF;
- guia de contribuição (`CONTRIBUTING.md`).

---

<a id="gestao"></a>
# 📊 Gestão do Projeto

O desenvolvimento é acompanhado pelas ferramentas de gerenciamento da equipe.

🔗 **Jira:** [Acessar Jira](https://csprj-adsr-2p-e2.atlassian.net/jira/software/c/projects/PI2/boards/2/timeline?selectedIssue=PI2-81)

🔗 **Trello:** [Acessar Trello](https://trello.com/b/t0u8Iplf)

**Última atualização:** 25/11/2026

---

<a id="equipe"></a>
# 👥 Equipe

| Integrante | Papel no Projeto | Principais Responsabilidades |
|---|---|---|
| Aline Santos | Game Designer, Desenvolvedora | UX/UI e Narrativa, Desenvolvimento e implementação do produto |
| Ana Rita Correia | Product Owner, Desenvolvedora | Visão do produto e priorização do backlog, Desenvolvimento e implementação do produto |
| Juan Nunes |  |  |
| Juliana Freire | Scrum Master | Organização do fluxo e acompanhamento das tarefas |
| Leandro Luís |  |  |
| Luiz Mario | Desenvolvedor | Desenvolvimento e implementação do produto |
| Vinícius Lino | Desenvolvedor | Desenvolvimento e implementação do produto |
| Vitória Santos | Game Designer, Desenvolvedor | UX/UI, Desenvolvimento e implementação do produto |

### Perfis

* 👤 **Aline Santos**: [GitHub](https://github.com/alines91) | [LinkedIn](https://www.linkedin.com/in/aline-santos-920488111/)
* 👤 **Ana Rita Correia**: [GitHub](https://github.com/anaritamoura) | [LinkedIn](https://www.linkedin.com/in/anaritamcorreia/en)
* 👤 **Gabriel Farias**: [GitHub](https://github.com/gabrielfariaxs) | [LinkedIn](https://www.linkedin.com/in/gabrielfariaxs)
* 👤 **Juan Nunes**: [GitHub](https://github.com/JuanSVJ60)
* 👤 **Juliana Freire**: [GitHub](https://github.com/julianafreire-jpg) | [LinkedIn](https://www.linkedin.com/in/juliana-freire-40393685/)
* 👤 **Leandro Luís**
* 👤 **Luiz Mario**: [GitHub](https://github.com/romaiziul) | [LinkedIn](https://www.linkedin.com/in/luiz-mario-moreira)
* 👤 **Vinícius Lino**: [GitHub](https://github.com/Vini-Lino)
* 👤 **Vitória Santos**: [GitHub](https://github.com/vitoriaSilva94) | [LinkedIn](https://www.linkedin.com/in/vitoria-rodrigues-493a57248/)

---

<a id="links"></a>
# 🔗 Links Importantes

* 📚 **Repositório de documentação e entregas:** [Acessar projeto2-squad2](https://github.com/Vini-Lino/projeto2-squad2)
* 🧩 **Épicos, histórias de usuário e diagramas:** [Consultar no projeto2-squad2](https://github.com/Vini-Lino/projeto2-squad2#epicos)
* 📘 **Guia de contribuição:** [Consultar CONTRIBUTING.md](https://github.com/Vini-Lino/projeto2-squad2/blob/main/CONTRIBUTING.md)
* 🎨 **Figma:** [Acessar no Figma](https://www.figma.com/board/YzFyw4c14ZIFgUF8Eu8Pjm/Projeto-Integrador---Cesar-2026.2?node-id=0-1&t=iAXRReHgs6IUaz6I-1)
* 📋 **Jira:** [Acessar Jira](https://csprj-adsr-2p-e2.atlassian.net/jira/software/c/projects/PI2/boards/2?atlOrigin=eyJpIjoiMWEwODM2MTk4OTJhNDA4NDliMWQ0OWIxOGRmMmU0NzciLCJwIjoiaiJ9)
* 📋 **Trello:** [Acessar Board](https://trello.com/b/t0u8Iplf)

---

<p align="center">
  Desenvolvido pela <strong>Equipe 2</strong>
</p>

<p align="center">
  CESAR School • ADS • 2026
</p>
