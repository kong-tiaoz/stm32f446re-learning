#Requires -Version 5.1
# Only compiles copied projects; no programmer or probe commands are used.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$projects = 'book-timers-CH11','book-adc-CH12','book-dac-CH13','book-i2c-CH14','book-low-power-CH19'
$results = @()
foreach ($project in $projects) {
    $directory = Join-Path $root $project
    $examples = Get-Content -Raw -LiteralPath (Join-Path $directory 'examples.json') | ConvertFrom-Json
    foreach ($example in $examples) {
        Write-Host "BUILD $project $($example.name)"
        & powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File (Join-Path $directory 'scripts/build.ps1') -Example $example.name *> (Join-Path $directory ('artifacts/verify-' + $example.name + '.log'))
        $code = $LASTEXITCODE
        $row = [pscustomobject]@{ Project = $project; Example = $example.name; Configuration = 'Debug'; ExitCode = $code }
        $results += $row
        Write-Host "RESULT $project $($example.name) $code"
        $results | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $root 'book-timers-CH11/artifacts/chapter-build-results.json') -Encoding UTF8
    }
}
if (@($results | Where-Object ExitCode -NE 0).Count -gt 0) { exit 1 }
