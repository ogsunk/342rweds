$content = Get-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Raw
$content = $content -replace "inline ", "static "
Set-Content "C:\Users\tyler\Desktop\Wizard External\esp_preview_array.txt" -Value $content
