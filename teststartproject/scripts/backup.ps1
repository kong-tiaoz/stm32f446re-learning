#Requires -Version 5.1
param([string]$SerialNumber)
. (Join-Path $PSScriptRoot 'common.ps1')
$selectedSerial = Select-StLink -SerialNumber $SerialNumber
$stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
$identityLog = New-ArtifactPath -Subdirectory 'logs' -Name ("target-$stamp.txt")
Assert-F446Target -SerialNumber $selectedSerial -LogPath $identityLog
$backupPath = New-ArtifactPath -Subdirectory 'backups' -Name ("flash-$stamp-$selectedSerial.bin")
$backupLog = New-ArtifactPath -Subdirectory 'logs' -Name ("backup-$stamp.txt")
$null = Invoke-Stm32Programmer -Arguments @('-c','port=SWD',"sn=$selectedSerial",'mode=UR','-u','0x08000000','0x80000',$backupPath) -LogPath $backupLog
if (-not (Test-Path -LiteralPath $backupPath) -or (Get-Item -LiteralPath $backupPath).Length -ne 524288) {
    throw 'Full Flash backup is missing or incomplete. Firmware must NOT be written.'
}
$hash = (Get-FileHash -LiteralPath $backupPath -Algorithm SHA256).Hash
$metadata = [pscustomobject]@{
    CreatedUtc = [DateTime]::UtcNow.ToString('o')
    SerialNumber = $selectedSerial
    Device = 'STM32F446xx'
    FlashBase = '0x08000000'
    FlashBytes = 524288
    File = $backupPath
    SHA256 = $hash
}
[IO.File]::WriteAllText(($backupPath + '.json'),($metadata | ConvertTo-Json),[Text.UTF8Encoding]::new($false))
Write-Host "Full Flash backup saved: $backupPath"
$metadata
