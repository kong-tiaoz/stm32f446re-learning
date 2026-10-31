#Requires -Version 5.1
param([ValidateSet('Debug','Release')][string]$Configuration = 'Debug', [ValidateSet('ex1','ex2')][string]$Example = 'ex1')
. (Join-Path $PSScriptRoot 'common.ps1')
$cmake = Find-Stm32Tool -Name 'cmake.exe' -Bundle 'cmake' -RelativePath 'bin\cmake.exe'
$ninja = Find-Stm32Tool -Name 'ninja.exe' -Bundle 'ninja' -RelativePath 'bin\ninja.exe'
$gcc = Find-Stm32Tool -Name 'arm-none-eabi-gcc.exe' -Bundle 'gnu-tools-for-stm32' -RelativePath 'bin\arm-none-eabi-gcc.exe'
$gccDirectory = Split-Path -Parent $gcc
$preset = $Configuration + '-' + $Example
$logDirectory = Join-Path $script:ProjectRoot 'artifacts'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
Start-Transcript -Path (Join-Path $logDirectory ($preset + '.log')) -Force | Out-Null
Push-Location $script:ProjectRoot
try {
    & $cmake --preset $preset "-DCMAKE_MAKE_PROGRAM=$ninja" "-DARM_GCC_BIN_DIR=$gccDirectory" | Out-Host
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
    & $cmake --build --preset $preset | Out-Host
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
} finally { Pop-Location; Stop-Transcript | Out-Null }
$buildDirectory = Join-Path $script:ProjectRoot ('build\' + $preset)
$firmware = 'book-dac-CH13-' + $Example
$binPath = Join-Path $buildDirectory ($firmware + '.bin')
$bytes = [IO.File]::ReadAllBytes($binPath)
if ($bytes.Length -lt 64 -or $bytes.Length -gt 524288) { throw 'Image must fit in the 512 KB F446RE Flash.' }
$initialStack = [BitConverter]::ToUInt32($bytes,0)
$resetVector = [BitConverter]::ToUInt32($bytes,4)
$sysTickVector = [BitConverter]::ToUInt32($bytes,60)
if ($initialStack -ne 0x20020000) { throw 'Initial stack is not the F446RE SRAM end.' }
foreach ($vector in @($resetVector,$sysTickVector)) {
    if (($vector -band 1) -ne 1 -or $vector -lt 0x08000001 -or $vector -ge (0x08000000 + $bytes.Length)) {
        throw 'Reset/SysTick vector is not Thumb code inside the generated Flash image.'
    }
}
Write-Host "Image checks passed: $($bytes.Length) bytes; stack=0x$($initialStack.ToString('X8'))."
[pscustomobject]@{
    Elf = Join-Path $buildDirectory ($firmware + '.elf')
    Hex = Join-Path $buildDirectory ($firmware + '.hex')
    Bin = $binPath
    Bytes = $bytes.Length
}
