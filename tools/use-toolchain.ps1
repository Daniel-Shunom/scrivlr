$compilerBin = Join-Path (Split-Path $PSScriptRoot -Parent) '.tools/gcc16/mingw64/bin'
if (-not (Test-Path (Join-Path $compilerBin 'g++.exe'))) {
    throw "Project GCC 16 toolchain is missing: $compilerBin"
}
$env:PATH = "$compilerBin;$env:PATH"
Remove-Item Env:C_INCLUDE_PATH, Env:CPLUS_INCLUDE_PATH -ErrorAction SilentlyContinue
& (Join-Path $compilerBin 'g++.exe') --version
