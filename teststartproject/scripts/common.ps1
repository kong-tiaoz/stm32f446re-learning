#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$script:ProjectRoot = Split-Path -Parent $PSScriptRoot

function Find-Stm32Tool {
    param([Parameter(Mandatory=$true)][string]$Name, [string]$Bundle, [string]$RelativePath)
    $command = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($null -ne $command) { return $command.Source }
    $localData = [Environment]::GetFolderPath('LocalApplicationData')
    $bundleRoot = Join-Path $localData ('stm32cube\bundles\' + $Bundle)
    if (Test-Path -LiteralPath $bundleRoot) {
        foreach ($version in (Get-ChildItem -LiteralPath $bundleRoot -Directory | Sort-Object Name -Descending)) {
            $candidate = Join-Path $version.FullName $RelativePath
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
    }
    throw "Cannot find $Name. Install/activate its bundle in the official STM32 VS Code extension."
}

function New-ArtifactPath {
    param([string]$Subdirectory, [string]$Name)
    $directory = Join-Path (Join-Path $script:ProjectRoot 'artifacts') $Subdirectory
    New-Item -ItemType Directory -Path $directory -Force | Out-Null
    return (Join-Path $directory $Name)
}

function Invoke-Stm32Programmer {
    param([string[]]$Arguments, [string]$LogPath)
    $programmer = Find-Stm32Tool -Name 'STM32_Programmer_CLI.exe' -Bundle 'programmer' -RelativePath 'bin\STM32_Programmer_CLI.exe'
    $lines = @(& $programmer @Arguments 2>&1)
    $nativeExitCode = $LASTEXITCODE
    $text = ($lines | ForEach-Object { $_.ToString() }) -join [Environment]::NewLine
    if ($LogPath) { [IO.File]::WriteAllText($LogPath, $text, [Text.UTF8Encoding]::new($false)) }
    Write-Host $text
    if ($nativeExitCode -ne 0) { throw "STM32CubeProgrammer failed (exit $nativeExitCode). Log: $LogPath" }
    return $text
}

function Select-StLink {
    param([string]$SerialNumber)
    if ($SerialNumber) {
        if ($SerialNumber -notmatch '^[A-Za-z0-9]+$') { throw 'Invalid ST-LINK serial number.' }
        return $SerialNumber
    }
    $text = Invoke-Stm32Programmer -Arguments @('-l','st-link')
    $probes = @([regex]::Matches($text, 'ST-LINK SN\s*:\s*([A-Za-z0-9]+)') | ForEach-Object { $_.Groups[1].Value })
    if ($probes.Count -ne 1) { throw "Found $($probes.Count) probes. Connect one board or specify -SerialNumber." }
    return $probes[0]
}

function Assert-F446Target {
    param([string]$SerialNumber, [string]$LogPath)
    $text = Invoke-Stm32Programmer -Arguments @('-c','port=SWD',"sn=$SerialNumber",'mode=UR') -LogPath $LogPath
    if ($text -notmatch 'Device ID\s*:\s*0x421\b' -or $text -notmatch 'Device name\s*:\s*STM32F446' -or $text -notmatch 'NVM size\s*:\s*512\s*KBytes') {
        throw 'Target is not the expected STM32F446xx with 512 KB Flash. No firmware was written.'
    }
}
