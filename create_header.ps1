$header = "#pragma once"
$array = Get-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Raw
$content = $header + [Environment]::NewLine + $array
Set-Content "C:\Users\tyler\Desktop\Wizard External\src\slate\resources\esp_preview_image.h" -Value $content
