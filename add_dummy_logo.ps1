$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Add dummy logobinary declaration after the commented include
$content = $content -replace '// #include "\.\./resources/images\.h"', '// #include "../resources/images.h"`n`nstatic unsigned char logobinary[1] = { 0 };'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Added dummy logobinary"
