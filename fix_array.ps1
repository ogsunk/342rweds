$raw = Get-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Raw
$fixed = $raw -replace ",\s*0x", ", 0x"
Set-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Value $fixed
