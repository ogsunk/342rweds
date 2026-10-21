$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Modify get_menu_logo_binary to return empty
$content = $content -replace 'const seasonal_logo_binary_t\& get_menu_logo_binary\(\) \{', 'const seasonal_logo_binary_t& get_menu_logo_binary() { static const seasonal_logo_binary_t empty{ nullptr, 0 }; return empty;'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Disabled logo function"
