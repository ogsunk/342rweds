$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Raw
$content = $content -replace 'inline unsigned char logobinary\[\] = \{', 'inline unsigned char logobinary[4286] = {'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Fixed logobinary array declaration"
