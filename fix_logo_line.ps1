(Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h') -replace 'inline unsigned char logobinary\[\] = \{', 'inline unsigned char logobinary[4286] = {' | Set-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
Write-Output "Fixed logobinary declaration"
