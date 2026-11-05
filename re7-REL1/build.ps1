Set-Location $PSScriptRoot

# compila todos os .c que estiverem dentro de src (em qualquer subpasta)
$fontes = Get-ChildItem src -Recurse -Filter *.c | ForEach-Object { $_.FullName }

# cada subpasta de src entra como pasta de #include (-I)
$pastas = @("-Isrc")
Get-ChildItem src -Recurse -Directory | ForEach-Object {
    $relativo = $_.FullName.Substring($PSScriptRoot.Length + 1).Replace('\', '/')
    $pastas += "-I$relativo"
}

gcc @fontes -o re7.exe `
    -IC:/raylib/raylib/src `
    @pastas `
    -LC:/raylib/raylib/src `
    -lraylib -lopengl32 -lgdi32 -lwinmm
