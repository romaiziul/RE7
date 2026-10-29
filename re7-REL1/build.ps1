Set-Location $PSScriptRoot

$fontes = Get-ChildItem src -Recurse -Filter *.c | ForEach-Object { $_.FullName }

gcc @fontes -o re7.exe `
    -IC:/raylib/raylib/src `
    -Isrc/core -Isrc/story -Isrc/minigames -Isrc/systems -Isrc/engine `
    -LC:/raylib/raylib/src `
    -lraylib -lopengl32 -lgdi32 -lwinmm
