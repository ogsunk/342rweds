$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Remove the dummy logobinary and restore images.h include
$content = $content -replace '// #include "\.\./resources/images\.h"`n`nstruct seasonal_logo_binary_t`n`n    static unsigned char logobinary\[1\] = \{ 0 \};', '#include "../resources/images.h"`n`n    struct seasonal_logo_binary_t'
$content = $content -replace 'static unsigned char logobinary\[1\] = \{ 0 \};', ''
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Restored images.h include"
