$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Uncomment the logo binary declarations
$content = $content -replace '// const seasonal_logo_binary_t logo_spring_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_spring_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_summer_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_summer_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_autumn_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_autumn_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_winter_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_winter_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_christmas_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_christmas_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_easter_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_easter_binary{ logobinary, sizeof(logobinary) };'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Uncommented logo binary declarations"
