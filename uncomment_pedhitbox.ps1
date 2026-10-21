$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Uncomment pedhitbox
$content = $content -replace '// ped_hitbox_photo\.load\(device, pedhitbox, sizeof\(pedhitbox\)\);', 'ped_hitbox_photo.load(device, pedhitbox, sizeof(pedhitbox));'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Uncommented pedhitbox"
