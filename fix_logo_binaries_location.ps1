$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Move dummy declarations before struct
$content = $content -replace '// #include "\.\./resources/images\.h"  // Disabled due to logo replacement issues`n`n    struct seasonal_logo_binary_t', '// #include "../resources/images.h"  // Disabled due to logo replacement issues`n`nstatic unsigned char dummy_logo[1] = { 0 };`n    struct seasonal_logo_binary_t'
$content = $content -replace 'const seasonal_logo_binary_t logo_spring_binary\{ dummy_logo, 1 \};', ''
$content = $content -replace 'const seasonal_logo_binary_t logo_summer_binary\{ dummy_logo, 1 \};', ''
$content = $content -replace 'const seasonal_logo_binary_t logo_autumn_binary\{ dummy_logo, 1 \};', ''
$content = $content -replace 'const seasonal_logo_binary_t logo_winter_binary\{ dummy_logo, 1 \};', ''
$content = $content -replace 'const seasonal_logo_binary_t logo_christmas_binary\{ dummy_logo, 1 \};', ''
$content = $content -replace 'const seasonal_logo_binary_t logo_easter_binary\{ dummy_logo, 1 \};', ''
# Add declarations in the right place
$content = $content -replace 'struct seasonal_logo_binary_t', 'struct seasonal_logo_binary_t`n`nconst seasonal_logo_binary_t logo_spring_binary{ dummy_logo, 1 };`nconst seasonal_logo_binary_t logo_summer_binary{ dummy_logo, 1 };`nconst seasonal_logo_binary_t logo_autumn_binary{ dummy_logo, 1 };`nconst seasonal_logo_binary_t logo_winter_binary{ dummy_logo, 1 };`nconst seasonal_logo_binary_t logo_christmas_binary{ dummy_logo, 1 };`nconst seasonal_logo_binary_t logo_easter_binary{ dummy_logo, 1 };'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Fixed logo binaries location"
