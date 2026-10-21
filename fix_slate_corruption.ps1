$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw

# Fix the corrupted struct definition on line 393
$content = $content -replace 'struct seasonal_logo_binary_t`n`nconst seasonal_logo_binary_t logo_spring_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_summer_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_autumn_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_winter_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_christmas_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_easter_binary\{ dummy_logo, 1 \};', 'struct seasonal_logo_binary_t {'

# Restore the commented out logo binary declarations
$content = $content -replace '// const seasonal_logo_binary_t logo_spring_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_spring_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_summer_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_summer_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_autumn_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_autumn_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_winter_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_winter_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_christmas_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_christmas_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_easter_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_easter_binary{ logobinary, sizeof(logobinary) };'

# Remove any dummy_logo references if they exist
$content = $content -replace 'static unsigned char dummy_logo\[1\] = \{ 0 \};', ''

$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Fixed slate.cpp corruption"
