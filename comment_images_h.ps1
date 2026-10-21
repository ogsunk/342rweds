$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Comment out the images.h include
$content = $content -replace '#include "\.\./resources/images\.h"', '// #include "../resources/images.h"'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Commented out images.h include"
