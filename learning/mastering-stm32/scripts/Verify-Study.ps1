#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$studyRoot = Split-Path -Parent $PSScriptRoot
$pandoc = (Get-Command pandoc.exe -CommandType Application -ErrorAction Stop).Source
$failures = [Collections.Generic.List[string]]::new()
$imageTotal = 0
$chapterTotal = 0
foreach ($chapter in Get-ChildItem -LiteralPath (Join-Path $studyRoot 'chapters') -Filter '*.md') {
    $markdown = [IO.File]::ReadAllText($chapter.FullName)
    if ($markdown -notmatch '(?m)^# [35]\. ' -or $markdown -notmatch 'SHA256' -or $markdown -notmatch 'id="source-p\d+"') {
        $failures.Add("Missing heading/source fingerprint/page anchor: $($chapter.Name)")
    }
    if ($markdown -notmatch '(?m)^``` c') { $failures.Add("No reconstructed C listing: $($chapter.Name)") }
    $renderPath = Join-Path (Join-Path $studyRoot '.local') ($chapter.BaseName + '-rendered.html')
    & $pandoc --from=gfm --to=html5 --wrap=none --output=$renderPath $chapter.FullName
    if ($LASTEXITCODE -ne 0) { throw "Markdown render failed: $($chapter.Name)" }
    $rendered = [IO.File]::ReadAllText($renderPath)
    $images = [regex]::Matches($rendered, '<img\b[^>]*src="([^"]+)"')
    foreach ($image in $images) {
        $path = Join-Path $chapter.DirectoryName ([Net.WebUtility]::HtmlDecode($image.Groups[1].Value))
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { $failures.Add("Missing image: $path") }
    }
    $imageTotal += $images.Count
    $chapterTotal++
    Write-Host "Rendered $($chapter.Name): $($images.Count) images."
}

$documents = @(Get-Item -LiteralPath (Join-Path $studyRoot 'README.md')) +
    @(Get-Item -LiteralPath (Join-Path $studyRoot 'contents.md')) +
    @(Get-ChildItem -LiteralPath (Join-Path $studyRoot 'chapters') -Filter '*.md') +
    @(Get-ChildItem -LiteralPath (Join-Path $studyRoot 'notes') -Filter '*.md')
$linkCount = 0
foreach ($document in $documents) {
    $text = [IO.File]::ReadAllText($document.FullName)
    foreach ($match in [regex]::Matches($text, '(?<!!)\[[^\]]*\]\(([^\)]+)\)')) {
        $target = $match.Groups[1].Value
        if ($target -match '^[a-z]+:') { continue }
        $parts = $target -split '#', 2
        $targetPath = if ($parts[0]) { Join-Path $document.DirectoryName ([Uri]::UnescapeDataString($parts[0])) } else { $document.FullName }
        if (-not (Test-Path -LiteralPath $targetPath -PathType Leaf)) { $failures.Add("Broken local link in $($document.Name): $target"); continue }
        if ($parts.Count -eq 2 -and $parts[1] -match '^source-p\d+$') {
            $linkedText = [IO.File]::ReadAllText($targetPath)
            if ($linkedText -notmatch ('id="' + [regex]::Escape($parts[1]) + '"')) { $failures.Add("Missing page anchor: $target") }
        }
        $linkCount++
    }
}
if ($chapterTotal -ne 2 -or $imageTotal -ne 39) { $failures.Add("Expected 2 initial chapters and 39 images; got $chapterTotal chapters, $imageTotal images.") }
if ($failures.Count) { throw ($failures -join [Environment]::NewLine) }
$report = [pscustomobject]@{ CheckedUtc=[DateTime]::UtcNow.ToString('o'); Chapters=$chapterTotal; RenderedImages=$imageTotal; LocalLinks=$linkCount; Failures=0 }
[IO.File]::WriteAllText((Join-Path (Join-Path $studyRoot '.local') 'verification.json'), ($report | ConvertTo-Json), [Text.UTF8Encoding]::new($false))
Write-Host "PASS: $chapterTotal chapters, $imageTotal rendered images, $linkCount local links/page anchors."
