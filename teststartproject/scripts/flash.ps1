#Requires -Version 5.1
param([string]$SerialNumber, [ValidateSet('Debug','Release')][string]$Configuration = 'Debug')
. (Join-Path $PSScriptRoot 'common.ps1')
$image = & (Join-Path $PSScriptRoot 'build.ps1') -Configuration $Configuration
$backup = & (Join-Path $PSScriptRoot 'backup.ps1') -SerialNumber $SerialNumber
if ((Get-Item -LiteralPath $backup.File).Length -ne 524288 -or (Get-FileHash -LiteralPath $backup.File -Algorithm SHA256).Hash -ne $backup.SHA256) {
    throw 'Backup integrity check failed. No firmware was written.'
}
$stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
$flashLog = New-ArtifactPath -Subdirectory 'logs' -Name ("flash-$stamp.txt")
$text = Invoke-Stm32Programmer -Arguments @('-c','port=SWD',"sn=$($backup.SerialNumber)",'mode=UR','-d',$image.Bin,'0x08000000','-v','-rst') -LogPath $flashLog
if ($text -notmatch 'Download verified successfully') { throw "Tool did not confirm verification. Inspect $flashLog" }
$result = [pscustomobject]@{
    CreatedUtc = [DateTime]::UtcNow.ToString('o')
    SerialNumber = $backup.SerialNumber
    Firmware = $image.Bin
    FirmwareSHA256 = (Get-FileHash -LiteralPath $image.Bin -Algorithm SHA256).Hash
    FirmwareBytes = $image.Bytes
    Backup = $backup.File
    BackupSHA256 = $backup.SHA256
    Log = $flashLog
    Verified = $true
}
$resultPath = New-ArtifactPath -Subdirectory 'logs' -Name ("flash-$stamp.json")
[IO.File]::WriteAllText($resultPath,($result | ConvertTo-Json),[Text.UTF8Encoding]::new($false))
Write-Host 'Programming and verification succeeded. Expected LD2 pattern: two short flashes, one long, every 2 s.'
$result
