# Read the single-line array and format it with newlines
$raw = Get-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Raw
# Insert newlines every 16 bytes for readability
$parts = [regex]::Split($raw, '(?<=0x[0-9A-Fa-f]{2}), ')
$formatted = ""
for ($i = 0; $i -lt $parts.Count; $i++) {
    if ($i -gt 0 -and $i % 16 -eq 0) {
        $formatted += "`r`n"
    }
    if ($i -lt $parts.Count - 1) {
        $formatted += $parts[$i] + ", "
    } else {
        $formatted += $parts[$i]
    }
}
Set-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Value $formatted
