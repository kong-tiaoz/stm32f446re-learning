#Requires -Version 5.1
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Convert-Book.ps1')
function Assert-True($Condition, $Message) { if (-not $Condition) { throw $Message } }

[xml]$fixture = '<html><body><p>previous chapter</p><p><a id="p106"/><b>3. Hello, Nucleo!</b></p><p><a id="p107"/><img src="figure.jpg"/></p><p>body</p><p><a id="p119"/>4. Next chapter</p><p>must not leak</p></body></html>'
$selection = @(Select-ChapterNodes -Document $fixture -StartAnchor 'p106' -EndAnchor 'p119')
Assert-True ($selection.Count -eq 3) 'Chapter boundary must include the first paragraph and exclude the next chapter.'
Assert-True (($selection | ForEach-Object OuterXml) -join '' -match 'figure.jpg') 'Image-only paragraphs must survive chapter selection.'
Assert-True (($selection | ForEach-Object InnerText) -join '' -notmatch 'previous|Next chapter|must not leak') 'Adjacent chapters must not leak into the chapter.'
Write-Host 'PASS: chapter boundaries and image-only paragraphs'

[xml]$listing = '<html><body><p>Filename: src/main.c</p><p>66</p><p>int main(void)</p><p>67</p><p>{</p><p>68</p><p>69</p><p>HAL_Delay(500);</p><p>70</p><p>}</p><p>The following paragraph is prose.</p></body></html>'
$blocks = @(Convert-NumberedListings -Nodes @($listing.html.body.ChildNodes))
$code = @($blocks | Where-Object LocalName -eq 'pre')
Assert-True ($code.Count -eq 1) 'A numbered listing must become a code block.'
Assert-True ($code[0].InnerText -match "int main\(void\)\r?\n\{\r?\n\r?\nHAL_Delay\(500\);") 'Blank numbered lines and code order must be preserved.'
Assert-True (($blocks | Where-Object LocalName -eq 'p' | ForEach-Object InnerText) -contains 'The following paragraph is prose.') 'Prose following a listing must remain outside the code block.'
Write-Host 'PASS: numbered C listings, blank lines, following prose'

[xml]$inline = '<html><body><p>For example, use the following code: <b>Filename: Core/Src/main.c</b></p><p>1</p><p>#include "main.h"</p><p>2</p><p>int main(void) {}</p></body></html>'
$inlineBlocks = @(Convert-NumberedListings -Nodes @($inline.html.body.ChildNodes))
Assert-True (@($inlineBlocks | Where-Object LocalName -eq 'pre').Count -eq 1) 'Listings introduced by an inline Filename caption must also become code blocks.'
Write-Host 'PASS: inline filename captions'
