#Requires -Version 5.1
param([ValidateSet('CH27-EX1','CH27-EX2')][string]$Example = 'CH27-EX1')
. (Join-Path $PSScriptRoot 'common.ps1')
$cmake = Find-Stm32Tool -Name 'cmake.exe' -Bundle 'cmake' -RelativePath 'bin\cmake.exe'
$ninja = Find-Stm32Tool -Name 'ninja.exe' -Bundle 'ninja' -RelativePath 'bin\ninja.exe'
$gcc = Find-Stm32Tool -Name 'arm-none-eabi-gcc.exe' -Bundle 'gnu-tools-for-stm32' -RelativePath 'bin\arm-none-eabi-gcc.exe'
$gccDirectory = Split-Path -Parent $gcc
Push-Location $script:ProjectRoot
try {
  & $cmake --preset $Example "-DCMAKE_MAKE_PROGRAM=$ninja" "-DARM_GCC_BIN_DIR=$gccDirectory" | Out-Host
  if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
  & $cmake --build --preset $Example --parallel 4 | Out-Host
  if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
} finally { Pop-Location }
Write-Host "ELF/HEX/BIN: $script:ProjectRoot\build\$Example\$Example.*"
