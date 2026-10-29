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
