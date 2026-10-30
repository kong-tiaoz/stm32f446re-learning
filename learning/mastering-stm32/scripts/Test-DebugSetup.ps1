#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$boardRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..\..')).Path
$testRoot = Join-Path (Split-Path -Parent $PSScriptRoot) ('.local\debug-setup-' + [Guid]::NewGuid().ToString('N'))
foreach ($project in @('book-hello-nucleo','book-debugging')) {
    $fixture = Join-Path $testRoot $project
    New-Item -ItemType Directory -Path (Join-Path $fixture 'scripts'),(Join-Path $fixture '.vscode') -Force | Out-Null
    foreach ($name in @('common.ps1','setup-debug.ps1')) {
        Copy-Item -LiteralPath (Join-Path (Join-Path $boardRoot $project) ('scripts\' + $name)) -Destination (Join-Path (Join-Path $fixture 'scripts') $name)
    }
    foreach ($name in @('settings.json','launch.template.json')) {
        Copy-Item -LiteralPath (Join-Path (Join-Path $boardRoot $project) ('.vscode\' + $name)) -Destination (Join-Path (Join-Path $fixture '.vscode') $name)
    }
    $launchPath = Join-Path $fixture '.vscode\launch.json'
    # A fresh clone has the template only; setup must create the local file.
    if (Test-Path -LiteralPath $launchPath) { throw 'Fixture unexpectedly contains launch.json.' }
    & (Join-Path $fixture 'scripts\setup-debug.ps1')
    if (-not (Test-Path -LiteralPath $launchPath)) { throw "$project setup did not create launch.json from the template." }
    $launch = Get-Content -LiteralPath $launchPath -Raw | ConvertFrom-Json
    $launch.configurations += [pscustomobject]@{name='Unrelated configuration';type='example';request='attach';marker='keep-me'}
    [IO.File]::WriteAllText($launchPath,($launch | ConvertTo-Json -Depth 10),[Text.UTF8Encoding]::new($false))
    & (Join-Path $fixture 'scripts\setup-debug.ps1')
    $actual = Get-Content -LiteralPath $launchPath -Raw | ConvertFrom-Json
    $config = @($actual.configurations | Where-Object type -eq 'cortex-debug')[0]
    foreach($field in @('armToolchainPath','gdbPath','serverpath','stm32cubeprogrammer')) {
        $path = $config.$field
        if($path -match '\$\{' -or -not (Test-Path -LiteralPath $path)) { throw "$project $field cannot resolve in a multi-root workspace without folder-scoped window settings: $path" }
    }
    if(@($actual.configurations | Where-Object { $_.PSObject.Properties['marker'] -and $_.marker -eq 'keep-me' }).Count -ne 1) { throw 'Setup removed an unrelated launch configuration.' }
    Write-Host "PASS: $project multi-root debug paths and preserved custom configuration"
}
