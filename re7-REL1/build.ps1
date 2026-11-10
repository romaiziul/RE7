param(
    # -Dev: build de desenvolvimento com o painel de testes (F10).
    # Gera re7_dev.exe e usa save_dev.dat. O build normal (sem -Dev) gera
    # re7.exe sem nenhum codigo do modo desenvolvedor.
    [switch]$Dev
)

Set-Location $PSScriptRoot

# escolha.c e um exemplo independente e tem a sua propria funcao main.
$exemploEscolha = Join-Path $PSScriptRoot 'src\escolha.c'
$fontes = Get-ChildItem src -Recurse -File -Filter *.c |
    Where-Object { $_.FullName -ne $exemploEscolha } |
    ForEach-Object { $_.FullName }

# cada subpasta de src entra como pasta de #include (-I)
$pastas = @("-Isrc")
Get-ChildItem src -Recurse -Directory | ForEach-Object {
    $relativo = $_.FullName.Substring($PSScriptRoot.Length + 1).Replace('\', '/')
    $pastas += "-I$relativo"
}

$saida = 're7.exe'
$extras = @()

if ($Dev) {
    $saida = 're7_dev.exe'
    $extras = @('-DRE7_DEV_MODE')
}

& "C:\raylib\w64devkit\bin\gcc.exe" @fontes -o $saida `
    @extras `
    -IC:/raylib/raylib/src `
    @pastas `
    -LC:/raylib/raylib/src `
    -lraylib -lopengl32 -lgdi32 -lwinmm
