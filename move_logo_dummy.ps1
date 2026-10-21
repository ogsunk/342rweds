$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Move the dummy logobinary to before the binary declarations
$content = $content -replace 'static unsigned char logobinary\[1\] = \{ 0 \};`n    struct seasonal_logo_binary_t', 'struct seasonal_logo_binary_t`n`n    static unsigned char logobinary[1] = { 0 };'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Moved dummy logobinary"
