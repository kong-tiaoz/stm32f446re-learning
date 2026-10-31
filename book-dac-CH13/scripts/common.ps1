#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$script:ProjectRoot = Split-Path -Parent $PSScriptRoot

function Find-Stm32Tool {
    param([Parameter(Mandatory=$true)][string]$Name, [string]$Bundle, [string]$RelativePath)
    $command = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($null -ne $command) { return $command.Source }
    $localDataRoots = @([Environment]::GetFolderPath('LocalApplicationData'), $env:LOCALAPPDATA)
    if ($env:USERPROFILE) { $localDataRoots += Join-Path $env:USERPROFILE 'AppData\Local' }
    foreach ($localData in ($localDataRoots | Where-Object { $_ } | Select-Object -Unique)) {
        $bundleRoot = Join-Path $localData ('stm32cube\bundles\' + $Bundle)
        if (Test-Path -LiteralPath $bundleRoot) {
            foreach ($version in (Get-ChildItem -LiteralPath $bundleRoot -Directory | Sort-Object Name -Descending)) {
                $candidate = Join-Path $version.FullName $RelativePath
                if (Test-Path -LiteralPath $candidate) { return $candidate }
            }
        }
    }
    throw "Cannot find $Name. Install/activate its bundle in the official STM32 VS Code extension."
}

