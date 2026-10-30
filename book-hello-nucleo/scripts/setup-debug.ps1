#Requires -Version 5.1
# This discovers local executable paths. It never connects to the target.
. (Join-Path $PSScriptRoot 'common.ps1')
$gcc = Find-Stm32Tool -Name 'arm-none-eabi-gcc.exe' -Bundle 'gnu-tools-for-stm32' -RelativePath 'bin\arm-none-eabi-gcc.exe'
$gdb = Find-Stm32Tool -Name 'arm-none-eabi-gdb.exe' -Bundle 'gnu-gdb-for-stm32' -RelativePath 'bin\arm-none-eabi-gdb.exe'
$server = Find-Stm32Tool -Name 'ST-LINK_gdbserver.exe' -Bundle 'stlink-gdbserver' -RelativePath 'bin\ST-LINK_gdbserver.exe'
$programmer = Find-Stm32Tool -Name 'STM32_Programmer_CLI.exe' -Bundle 'programmer' -RelativePath 'bin\STM32_Programmer_CLI.exe'
$launchPath = Join-Path $script:ProjectRoot '.vscode\launch.json'
if (-not (Test-Path -LiteralPath $launchPath)) {
    Copy-Item -LiteralPath (Join-Path $script:ProjectRoot '.vscode\launch.template.json') -Destination $launchPath
}
$launch = Get-Content -LiteralPath $launchPath -Raw | ConvertFrom-Json
$targets = @($launch.configurations | Where-Object { $_.type -eq 'cortex-debug' -and $_.name -eq 'Attach STM32F446RE (no download)' })
if ($targets.Count -ne 1) { throw 'Expected exactly one Attach STM32F446RE (no download) configuration.' }
$values = @{
    'armToolchainPath' = (Split-Path -Parent $gcc).Replace('\','/')
    'gdbPath' = $gdb.Replace('\','/')
    'serverpath' = $server.Replace('\','/')
    'stm32cubeprogrammer' = (Split-Path -Parent $programmer).Replace('\','/')
}
foreach ($entry in $values.GetEnumerator()) {
    $targets[0] | Add-Member -NotePropertyName $entry.Key -NotePropertyValue $entry.Value -Force
}
[IO.File]::WriteAllText($launchPath,($launch | ConvertTo-Json -Depth 20),[Text.UTF8Encoding]::new($false))
# Window-scoped extension settings are ignored at folder level in multi-root workspaces.
$settingsPath = Join-Path $script:ProjectRoot '.vscode\settings.json'
$settings = Get-Content -LiteralPath $settingsPath -Raw | ConvertFrom-Json
foreach ($name in @('cortex-debug.armToolchainPath','cortex-debug.gdbPath','cortex-debug.stlinkPath','cortex-debug.stm32cubeprogrammer')) {
    $settings.PSObject.Properties.Remove($name)
}
[IO.File]::WriteAllText($settingsPath,($settings | ConvertTo-Json -Depth 20),[Text.UTF8Encoding]::new($false))
Write-Host 'Cortex-Debug paths configured directly in launch.json for single-folder and multi-root workspaces. F5 attaches without downloading.'
