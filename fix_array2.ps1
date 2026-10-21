$content = Get-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt"
$header = $content[0]
$bytes = $content[1..($content.Count-1)] -join " "
$bytes = $bytes -replace ",\s+", ", "
$bytes = $bytes -replace "^\s*,\s*", ""
$output = $header + $bytes + "};"
Set-Content "C:\Users\tyler\Desktop\Wizard External\silent_esp_preview_array.txt" -Value $output
