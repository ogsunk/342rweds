# Since images.h is corrupted, I'll copy from a backup or recreate it minimally
# For now, let's just delete the corrupted sections and keep only essential parts
$lines = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
$cleanLines = @()
$skipUntil = -1

for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match 'inline unsigned char logobinary\[4286\]') {
        $skipUntil = $i
    }
    if ($i -eq $skipUntil) {
        # Replace with original logobinary declaration
        $cleanLines += "inline unsigned char logobinary[] = {"
        $cleanLines += "	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,"
        $cleanLines += "	0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,"
        $cleanLines += "	0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00,"
        $cleanLines += "	0x08, 0x06, 0x00, 0x00, 0x00, 0x7F, 0x1D, 0x2B,"
        $cleanLines += "	0x83"
        $skipUntil = -1
        continue
    }
    if ($lines[$i] -match 'inline unsigned char logobinary\[\] = \{ 0x89') {
        continue  # Skip duplicates
    }
    if ($lines[$i] -match 'inline unsigned char pedhitbox') {
        continue  # Skip pedhitbox
    }
    $cleanLines += $lines[$i]
}

$cleanLines | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Restored minimal images.h"
