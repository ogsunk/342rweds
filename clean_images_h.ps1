# Read the file and remove broken logobinary redefinitions
$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Raw

# Remove the broken minimal logobinary (8 bytes) and keep the original
$content = $content -replace 'inline unsigned char logobinary\[\] = \{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A \};', ''

# Fix any remaining broken logobinary with [4286] or [] that are duplicates
# Keep only the first occurrence which should be the original
$lines = $content -split "`n"
$seenLogobinary = $false
$cleanLines = @()

foreach ($line in $lines) {
    if ($line -match 'inline unsigned char logobinary') {
        if (-not $seenLogobinary) {
            $cleanLines += $line
            $seenLogobinary = $true
        }
        # Skip duplicates
    } else {
        $cleanLines += $line
    }
}

$cleanLines -join "`n" | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Cleaned images.h"
