# Read the header file
$content = Get-Content 'C:\Users\tyler\Documents\future\future\SunketsWare-FiveM\SunketsWare-FiveM\src\Ext\Includes\CustomWidgets\silent_esp_texture.h' -Raw

# Find the start and end of the array
$startIndex = $content.IndexOf('inline unsigned char silent_esp[] = {')
if ($startIndex -eq -1) {
    Write-Host "Could not find silent_esp array start"
    exit 1
}

$startIndex += 'inline unsigned char silent_esp[] = {'.Length
$endIndex = $content.IndexOf('};', $startIndex)
if ($endIndex -eq -1) {
    Write-Host "Could not find silent_esp array end"
    exit 1
}

$hexData = $content.Substring($startIndex, $endIndex - $startIndex)

# Parse hex values
$bytes = [System.Collections.Generic.List[byte]]::new()
$lines = $hexData -split '\n'
foreach ($line in $lines) {
    $line = $line.Trim()
    if ($line -and -not $line.StartsWith('//')) {
        $line = $line -replace ',$', ''
        $values = $line -split ','
        foreach ($val in $values) {
            $val = $val.Trim()
            if ($val) {
                try {
                    # Remove 0x prefix if present
                    $val = $val -replace '^0x', ''
                    $bytes.Add([byte]::Parse($val, 'HexNumber'))
                } catch {
                    Write-Host "Skipping invalid value: $val"
                }
            }
        }
    }
}

Write-Host "Extracted $($bytes.Count) bytes"

# Write to PNG file
$outputPath = 'C:\Users\tyler\Desktop\Wizard External\silent_esp_preview.png'
[System.IO.File]::WriteAllBytes($outputPath, $bytes.ToArray())

Write-Host "Saved to: $outputPath"

# Check PNG signature
$sig = $bytes[0..7]
$expectedSig = @(0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A)
$sigStr = $sig -join ','
$expectedStr = $expectedSig -join ','
if ($sigStr -eq $expectedStr) {
    Write-Host "Valid PNG signature detected"
} else {
    Write-Host "WARNING: Invalid PNG signature"
    Write-Host "First 8 bytes: $sigStr"
}
