[byte[]]$bytes = [System.IO.File]::ReadAllBytes("C:\Users\tyler\Desktop\Wizard External\silent_esp_preview.png")
$hexStrings = $bytes | ForEach-Object { "0x{0:X2}" -f $_ }
$header = "inline unsigned char silent_esp_preview[$($bytes.Count)] = {"
$byteString = $hexStrings -join ", "
$footer = "};"
$header + $byteString + $footer | Set-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt"
Write-Host "Converted $($bytes.Count) bytes to C array format"
