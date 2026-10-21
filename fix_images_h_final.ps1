$lines = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
$cleanLines = @()
$skip = $false

for ($i = 0; $i -lt $lines.Count; $i++) {
    $line = $lines[$i]
    # Skip the broken minimal logobinary[4286] and any broken pedhitbox
    if ($line -match 'inline unsigned char logobinary\[4286\]') {
        $skip = $true
        continue
    }
    if ($line -match 'inline unsigned char pedhitbox') {
        $skip = $true
        continue
    }
    if ($skip -and $line -match '\};') {
        $skip = $false
        continue
    }
    if (-not $skip) {
        $cleanLines += $line
    }
}

$cleanLines | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Fixed images.h"
