
Set-Location $PSScriptRoot

$gcc = "C:\Users\anarc\Downloads\w64devkit\bin\gcc.exe"

$raylib = "C:\Users\anarc\Downloads\raylib-6.2_win64_mingw-w64\raylib-6.2_win64_mingw-w64"

$fontes = Get-ChildItem src -Recurse -Filter *.c |
    Where-Object { $_.Name -ne "escolha.c" } |
    ForEach-Object { $_.FullName }

$pastas = @("-Isrc")

Get-ChildItem src -Recurse -Directory | ForEach-Object {
    $relativo = $_.FullName.Substring($PSScriptRoot.Length + 1).Replace('\', '/')
    $pastas += "-I$relativo"
}

& $gcc @fontes -o re7.exe `
    "-I$raylib\include" `
    @pastas `
    "-L$raylib\lib" `
    -lraylib -lopengl32 -lgdi32 -lwinmm

if ($LASTEXITCODE -ne 0) {
    Write-Host "Falha na compilacao!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Compilacao concluida!" -ForegroundColor Green
