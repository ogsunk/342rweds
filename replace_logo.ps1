$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\sunketsware_bytes.txt' -Raw
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\sunketsware_bytes_full.txt' -Encoding ASCII
