$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Raw
# Remove the broken pedhitbox and fix logobinary
$content = $content -replace 'inline unsigned char logobinary\[4286\] = \{[\s\S]*?\};', 'inline unsigned char logobinary[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };'
$content = $content -replace 'inline unsigned char pedhitbox\[\] = \{[\s\S]*?\};', ''
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Fixed images.h to minimal state"
