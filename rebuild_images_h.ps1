# Read the original sunketsware.png bytes
$logoBytes = [System.IO.File]::ReadAllBytes('C:\Users\tyler\Desktop\Wizard External\src\slate\resources\sunketsware.png')

# Find where logobinary was originally (before pedhitbox)
$lines = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
$newLines = @()

$foundLogobinary = $false
$inLogobinary = $false

foreach ($line in $lines) {
    if ($line -match 'inline unsigned char logobinary' -and -not $foundLogobinary) {
        $foundLogobinary = $true
        $newLines += "inline unsigned char logobinary[$($logoBytes.Count)] = {"
        for($i = 0; $i -lt $logoBytes.Count; $i += 16) {
            $chunk = $logoBytes[$i..[Math]::Min($i+15, $logoBytes.Count-1)]
            $hex = ($chunk | ForEach-Object { "0x{0:X2}" -f $_ }) -join ', '
            $newLines += "	$hex"
        }
        $newLines += "};"
        # Skip the old logobinary lines
        $inLogobinary = $true
    } elseif ($inLogobinary -and $line -match '\};') {
        $inLogobinary = $false
    } elseif (-not $inLogobinary) {
        $newLines += $line
    }
}

$newLines | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Rebuilt images.h with new logo"
