# Re:7

Jogo em C com raylib, projeto da CESAR School.

## Compilar e rodar

Requer gcc e raylib em `C:/raylib/raylib/src`. Na raiz desta pasta:

```powershell
.\build.ps1
.\re7.exe
```

O jogo carrega assets por caminho relativo, então o `re7.exe` deve ser executado da raiz desta pasta.

## Estrutura

```
src/
├─ main.c
├─ core/        game, scenes
├─ story/       story
├─ minigames/   minigame
├─ systems/     room
└─ engine/      ui, assets, audio_manager
assets/
├─ comum/       interface, menu e personagens
├─ ato1/ ato2/ ato3/ epilogo/
├─ music/       trilhas
└─ sounds/      efeitos
docs/           documentação de áudio
```

## Documentação

https://docs.google.com/document/d/1z7lnUvPvSEYXRe7CravEMiyw6phrurek2HpP9oDs34U/edit

## Testes

Regras do Ato 3 e do desafio "Entre as Linhas" (sem janela, sem raylib):

```powershell
gcc testes/teste_ato3.c src/atos/ato3/ato3_logica.c src/systems/tokens.c -Isrc/core -Isrc/systems -Isrc/atos/ato3 -o teste_ato3.exe
.\teste_ato3.exe
```

`testes/captura_telas.c` gera imagens das telas do Ato 3 e do painel de desenvolvedor (instruções no topo do arquivo).

## Modo desenvolvedor

Painel para navegar entre os atos e testar cenários. Existe só no build de desenvolvimento:

```powershell
.\build.ps1 -Dev
.\re7_dev.exe
```

- F10 abre e fecha o painel (abas Navegação, Tokens, Variáveis e Testes).
- F6 pula a fala atual, F7 avança para a próxima etapa (fim da cena ou do minijogo), F8 reinicia o ato atual.
- O build de desenvolvimento salva em `save_dev.dat`; o `save.dat` da jogadora não é usado.

O build normal (`.\build.ps1`, gera `re7.exe`) não define `RE7_DEV_MODE`: o painel e os atalhos não são compilados.