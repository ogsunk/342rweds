# Read PNG file
$pngPath = "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview.png"
$outputPath = "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt"

$bytes = [System.IO.File]::ReadAllBytes($pngPath)
$hexList = @()
for ($i = 0; $i -lt $bytes.Count; $i++) {
    $hexList += "0x{0:X2}" -f $bytes[$i]
}

$header = "inline unsigned char silent_esp_preview[$($bytes.Count)] = {"
$byteString = $hexList -join ", "
$footer = "};"

$header + $byteString + $footer | Set-Content $outputPath
Write-Host "Converted $($bytes.Count) bytes to C array format"
