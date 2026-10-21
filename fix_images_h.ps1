$lines = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
$lines[343257] = "inline unsigned char logobinary[4286] = {"
$lines | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
Write-Output "Fixed line 343258"
