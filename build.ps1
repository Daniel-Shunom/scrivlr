[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [switch]$Test,
    [switch]$NoPackage,
    [ValidateRange(1, 256)]
    [int]$Parallel = [Math]::Max(1, [Environment]::ProcessorCount)
)

$ErrorActionPreference = 'Stop'
$buildPath = Join-Path $PSScriptRoot "build/$($Configuration.ToLowerInvariant())"
$compilerBin = Join-Path $PSScriptRoot '.tools/gcc16/mingw64/bin'
$compilerOptions = @()
$savedEnvironment = @{}
if (Test-Path (Join-Path $compilerBin 'g++.exe')) {
    $buildPath = Join-Path $PSScriptRoot "build/gcc16/$($Configuration.ToLowerInvariant())"
    $compilerOptions = @("-DCMAKE_C_COMPILER=$compilerBin/gcc.exe", "-DCMAKE_CXX_COMPILER=$compilerBin/g++.exe")
    foreach ($name in @('PATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH')) {
        $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
    }
    $env:PATH = "$compilerBin;$env:PATH"
    Remove-Item Env:C_INCLUDE_PATH, Env:CPLUS_INCLUDE_PATH -ErrorAction SilentlyContinue
}
$testing = if ($Test) { 'ON' } else { 'OFF' }

function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

try {
    foreach ($program in @('cmake', 'ninja', 'cpack')) {
        if (-not (Get-Command $program -ErrorAction SilentlyContinue)) {
            throw "$program is required on PATH. Also install GCC 16+ with g++."
        }
    }
    Invoke-Checked 'cmake' (@('-S', $PSScriptRoot, '-B', $buildPath, '-G', 'Ninja',
        "-DCMAKE_BUILD_TYPE=$Configuration", "-DBUILD_TESTING=$testing") + $compilerOptions)
    Invoke-Checked 'cmake' @('--build', $buildPath, '--parallel', "$Parallel")
    if ($Test) {
        Invoke-Checked 'ctest' @('--test-dir', $buildPath, '--output-on-failure')
    }
    if (-not $NoPackage) {
        Invoke-Checked 'cpack' @('--config', (Join-Path $buildPath 'CPackConfig.cmake'), '-C', $Configuration)
        Write-Host "Packages: $(Join-Path $buildPath 'packages')"
    }
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
} finally {
    foreach ($name in $savedEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process')
    }
}
