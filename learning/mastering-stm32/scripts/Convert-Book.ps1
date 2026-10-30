#Requires -Version 5.1
param([string]$BookPath, [string]$PandocPath)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Select-ChapterNodes {
    param([xml]$Document, [string]$StartAnchor, [string]$EndAnchor)
    $inside = $false
    foreach ($node in $Document.DocumentElement.SelectSingleNode('*[local-name()="body"]').ChildNodes) {
        $ids = @($node.SelectNodes('.//*[@id]') | ForEach-Object { $_.GetAttribute('id') })
        if ($ids -contains $EndAnchor) { break }
        if ($ids -contains $StartAnchor) { $inside = $true }
        if ($inside -and $node.NodeType -eq 'Element') { $node.CloneNode($true) }
    }
    if (-not $inside) { throw "Missing chapter anchor: $StartAnchor" }
}

function Convert-NumberedListings {
    param([System.Xml.XmlNode[]]$Nodes)
    $lines = [Collections.Generic.List[string]]::new()
    $owner = if ($Nodes.Count) { $Nodes[0].OwnerDocument } else { $null }
    $allowStart = $false
    $expected = -1
    for ($i = 0; $i -lt $Nodes.Count; $i++) {
        $node = $Nodes[$i]
        $text = $node.InnerText.Trim()
        $number = 0
        $isNumber = [int]::TryParse($text, [ref]$number)
        if ($isNumber -and ($allowStart -or $number -eq $expected)) {
            $allowStart = $false
            $expected = $number + 1
            $nextNumber = 0
            if ($i + 1 -lt $Nodes.Count -and [int]::TryParse($Nodes[$i + 1].InnerText.Trim(), [ref]$nextNumber)) {
                $lines.Add('')
            } elseif ($i + 1 -lt $Nodes.Count) {
                $lines.Add($Nodes[++$i].InnerText.TrimEnd())
            }
            continue
        }
        if ($lines.Count) {
            $pre = $owner.CreateElement('pre')
            $code = $owner.CreateElement('code')
            $code.SetAttribute('class', 'language-c')
            $code.InnerText = ($lines -join "`n") + "`n"
            [void]$pre.AppendChild($code)
            $pre
            $lines.Clear()
        }
        if ($text -match '\bFilename:') { $allowStart = $true; $expected = -1 }
        $node
    }
    if ($lines.Count) {
        $pre = $owner.CreateElement('pre')
        $code = $owner.CreateElement('code')
        $code.SetAttribute('class', 'language-c')
        $code.InnerText = ($lines -join "`n") + "`n"
        [void]$pre.AppendChild($code)
        $pre
    }
}

function Read-EpubText($Archive, [string]$Name) {
    $entry = $Archive.GetEntry($Name)
    if ($null -eq $entry) { throw "Missing EPUB member: $Name" }
    $reader = [IO.StreamReader]::new($entry.Open())
    try { return $reader.ReadToEnd() } finally { $reader.Dispose() }
}

function Get-TocLabel($Point) { return $Point.SelectSingleNode('./*[local-name()="navLabel"]/*[local-name()="text"]').InnerText }
function Get-TocSource($Point) { return $Point.SelectSingleNode('./*[local-name()="content"]').GetAttribute('src') }
function Normalize-Heading([string]$Text) { return (($Text -replace '^\s*\d+(?:\.\d+)*[.)]?\s+', '') -replace '\s+', ' ').Trim() }
function Write-Utf8([string]$Path, [string]$Text) { [IO.File]::WriteAllText($Path, $Text, [Text.UTF8Encoding]::new($false)) }

function Invoke-BookConversion {
    param([string]$BookPath, [string]$PandocPath)
    $studyRoot = Split-Path -Parent $PSScriptRoot
    if (-not $BookPath) {
        $library = Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'zlibrary'
        $books = @(Get-ChildItem -LiteralPath $library -Filter '*Mastering STM32*2nd*.epub' -File)
        if ($books.Count -ne 1) { throw 'Specify -BookPath pointing to your EPUB.' }
        $BookPath = $books[0].FullName
    }
    $BookPath = (Get-Item -LiteralPath $BookPath).FullName
    if (-not $PandocPath) { $PandocPath = (Get-Command pandoc.exe -CommandType Application -ErrorAction Stop).Source }
    foreach ($directory in @('chapters', 'assets', '.local')) {
        New-Item -ItemType Directory -Path (Join-Path $studyRoot $directory) -Force | Out-Null
    }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $archive = [IO.Compression.ZipFile]::OpenRead($BookPath)
    try {
        [xml]$toc = Read-EpubText $archive 'toc.ncx'
        $allPoints = @($toc.SelectNodes('//*[local-name()="navPoint"]'))
        $selected = @(
            @{ Title = 'Hello, Nucleo!'; File = '03-hello-nucleo.md'; Start = 'p106'; End = 'p119'; Html = 'index_split_001.html'; Project = 'book-hello-nucleo' },
            @{ Title = 'Introduction to Debugging'; File = '05-introduction-to-debugging.md'; Start = 'p147'; End = 'p159'; Html = 'index_split_002.html'; Project = 'book-debugging' }
        )
        $bookHash = (Get-FileHash -LiteralPath $BookPath -Algorithm SHA256).Hash
        $reports = @()
        foreach ($chapter in $selected) {
            [xml]$source = Read-EpubText $archive $chapter.Html
            $nodes = @(Select-ChapterNodes -Document $source -StartAnchor $chapter.Start -EndAnchor $chapter.End)
            $point = @($allPoints | Where-Object { (Get-TocLabel $_) -eq $chapter.Title })[0]
            $sectionPoints = @($point) + @($point.SelectNodes('.//*[local-name()="navPoint"]'))
            $headingLabels = @($sectionPoints | ForEach-Object { Normalize-Heading (Get-TocLabel $_) })
            $html = [xml]'<html><head><meta charset="utf-8"/></head><body/></html>'
            $body = $html.DocumentElement.SelectSingleNode('body')
            $headingCount = 0
            $imageNames = [Collections.Generic.HashSet[string]]::new()
            $previousPlain = ''
            foreach ($node in $nodes) {
                $copy = $html.ImportNode($node, $true)
                foreach ($element in @($copy) + @($copy.SelectNodes('.//*'))) {
                    foreach ($attribute in @($element.Attributes)) {
                        if ($attribute.Name -notin @('id', 'href', 'src', 'alt', 'xmlns')) { $element.RemoveAttributeNode($attribute) | Out-Null }
                    }
                    if ($element.HasAttribute('id')) { $element.SetAttribute('id', 'source-' + $element.GetAttribute('id')) }
                    if ($element.LocalName -eq 'img') {
                        $original = $element.GetAttribute('src')
                        $imageName = [IO.Path]::GetFileName($original)
                        if ($imageNames.Add($imageName)) {
                            $imageEntry = $archive.GetEntry($original)
                            if ($null -eq $imageEntry) { throw "Missing image: $original" }
                            [IO.Compression.ZipFileExtensions]::ExtractToFile($imageEntry, (Join-Path (Join-Path $studyRoot 'assets') $imageName), $true)
                        }
                        $element.SetAttribute('src', '../assets/' + $imageName)
                    }
                    if ($element.HasAttribute('href')) {
                        $href = $element.GetAttribute('href')
                        if ($href -match '^(?:index_split_\d+\.html)?#p(\d+)$') {
                            $page = [int]$Matches[1]
                            $destination = @($selected | Where-Object { $page -ge [int]($_.Start.Substring(1)) -and $page -lt [int]($_.End.Substring(1)) })
                            if ($destination.Count) { $element.SetAttribute('href', $destination[0].File + '#source-p' + $page) }
                            else { $element.SetAttribute('href', '../contents.md') }
                        }
                    }
                }
                $plain = $copy.InnerText.Trim()
                $normalized = Normalize-Heading $plain
                $isPageNumber = $plain -match '^\d+$' -and $previousPlain -eq $chapter.Title
                $previousPlain = $plain
                if ($headingLabels -contains $normalized -and $plain -match '^\d+(?:\.\d+)*\.?\s') {
                    $level = if ($plain -match '^\d+\.\d+\.\d+') { 3 } elseif ($plain -match '^\d+\.\d+') { 2 } else { 1 }
                    $heading = $html.CreateElement("h$level")
                    foreach ($anchor in @($copy.SelectNodes('.//*[local-name()="a"][@id]'))) { [void]$body.AppendChild($anchor.CloneNode($true)) }
                    $heading.InnerText = $plain
                    [void]$body.AppendChild($heading)
                    $headingCount++
                } elseif ($plain -eq $chapter.Title -or $isPageNumber) {
                    # Repeated running page headers / printed page numbers, not listing line numbers.
                    foreach ($anchor in @($copy.SelectNodes('.//*[local-name()="a"][@id]'))) { [void]$body.AppendChild($anchor.CloneNode($true)) }
                } else { [void]$body.AppendChild($copy) }
            }
            $converted = @(Convert-NumberedListings -Nodes @($body.ChildNodes))
            $body.RemoveAll()
            foreach ($node in $converted) { [void]$body.AppendChild($node) }
            $htmlPath = Join-Path (Join-Path $studyRoot '.local') ($chapter.File -replace '\.md$', '.html')
            Write-Utf8 $htmlPath $html.OuterXml
            $markdownPath = Join-Path (Join-Path $studyRoot 'chapters') $chapter.File
            & $PandocPath --from=html --to=gfm --wrap=none --output=$markdownPath $htmlPath
            if ($LASTEXITCODE -ne 0) { throw "Pandoc failed for $($chapter.Title)" }
            $text = [IO.File]::ReadAllText($markdownPath)
            $header = @"
<!-- Local study copy; author: Carmine Noviello. Preserve attribution. -->
> **原文学习副本**：Mastering STM32 — Second Edition / Carmine Noviello。
> EPUB：$([IO.Path]::GetFileName($BookPath))；SHA256：``$bookHash``。
> 原始位置：``$($chapter.Html)#$($chapter.Start)`` 至下一章 ``#$($chapter.End)`` 之前（EPUB 内部页锚点，非纸书印刷页码）。
> 配套可运行工程：[项目说明](../../../$($chapter.Project)/README.md)。[全书目录](../contents.md) · [学习记录](../notes/01-first-session.md)。
>
> 本副本由 PDF 转制 EPUB 整理而来；正文保留原作者表述及旧版界面说明。编号代码恢复为代码块，缩进与跨页断行不保证等同作者源文件；编译请使用独立工程。当前 VS Code 操作见学习记录。

"@
            Write-Utf8 $markdownPath ($header + "`n" + $text)
            $reports += [pscustomobject]@{ Title=$chapter.Title; Markdown=$chapter.File; Source=$chapter.Html + '#' + $chapter.Start; EndBefore=$chapter.End; Headings=$headingCount; Images=$imageNames.Count; CodeBlocks=@($converted | Where-Object LocalName -eq 'pre').Count; Characters=$text.Length }
        }
        $catalogue = [Collections.Generic.List[string]]::new()
        $catalogue.Add('# Mastering STM32 — 全书目录')
        $catalogue.Add('')
        $catalogue.Add('来源：当前本地 EPUB 的 toc.ncx。带链接的章节已整理；其余保留目录，随学习进度转换。纸书和 EPUB 页码不一定一致。')
        $catalogue.Add('')
        foreach ($top in $toc.SelectNodes('//*[local-name()="navMap"]/*[local-name()="navPoint"]')) {
            $catalogue.Add('## ' + (Get-TocLabel $top))
            $catalogue.Add('')
            $points = @($top.SelectNodes('.//*[local-name()="navPoint"]'))
            foreach ($p in $points) {
                $label = Get-TocLabel $p
                $depth = 0; $parent = $p.ParentNode
                while ($parent -ne $top -and $parent.LocalName -eq 'navPoint') { $depth++; $parent = $parent.ParentNode }
                $owner = $p
                while ($owner.ParentNode -ne $top -and $owner.ParentNode.LocalName -eq 'navPoint') { $owner = $owner.ParentNode }
                $match = @($selected | Where-Object { $_.Title -eq (Get-TocLabel $owner) })
                $sourceRef = Get-TocSource $p
                if ($match.Count -and $sourceRef -match '#(p\d+)$') { $line = '[' + $label + '](chapters/' + $match[0].File + '#source-' + $Matches[1] + ')' }
                else { $line = $label }
                $catalogue.Add(('  ' * $depth) + '- ' + $line)
            }
            $catalogue.Add('')
        }
        Write-Utf8 (Join-Path $studyRoot 'contents.md') ($catalogue -join "`n")
        $manifest = [pscustomobject]@{ BookPath=$BookPath; BookSHA256=$bookHash; ConvertedUtc=[DateTime]::UtcNow.ToString('o'); Chapters=$reports }
        Write-Utf8 (Join-Path (Join-Path $studyRoot '.local') 'conversion.json') ($manifest | ConvertTo-Json -Depth 5)
        $reports | Format-Table -AutoSize | Out-Host
    } finally { $archive.Dispose() }
}

if ($MyInvocation.InvocationName -ne '.') { Invoke-BookConversion -BookPath $BookPath -PandocPath $PandocPath }
