# Read the PNG file
$pngPath = "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview.png"
$outputPath = "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt"

$bytes = [System.IO.File]::ReadAllBytes($pngPath)
$byteCount = $bytes.Count

# Build C array string
$lines = @()
$line = "inline unsigned char silent_esp_preview[$byteCount] = {"
$lines += $line

$lineContent = ""
for ($i = 0; $i -lt $byteCount; $i++) {
    $hex = "0x{0:X2}" -f $bytes[$i]
    if ($i -eq 0) {
        $lineContent = $hex
    } else {
        $lineContent += ", " + $hex
    }
    
    # Break lines every 16 bytes
    if (($i + 1) % 16 -eq 0) {
        $lines += $lineContent + ","
        $lineContent = ""
    }
}

# Add remaining bytes
if ($lineContent -ne "") {
    $lines += $lineContent
}

$lines += "};"

# Write to file
$lines | Out-File -FilePath $outputPath -Encoding UTF8

Write-Host "Converted $byteCount bytes to C array format"
Write-Host "Output: $outputPath"
