$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Restore images.h include and remove all logo-related dummy code
$content = $content -replace '// #include "\.\./resources/images\.h"  // Disabled due to logo replacement issues`n`nstatic unsigned char dummy_logo\[1\] = \{ 0 \};`n    struct seasonal_logo_binary_t`n`nconst seasonal_logo_binary_t logo_spring_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_summer_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_autumn_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_winter_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_christmas_binary\{ dummy_logo, 1 \};`nconst seasonal_logo_binary_t logo_easter_binary\{ dummy_logo, 1 \};', '#include "../resources/images.h"`n`n    struct seasonal_logo_binary_t'
$content = $content -replace '// const seasonal_logo_binary_t logo_spring_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_spring_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_summer_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_summer_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_autumn_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_autumn_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_winter_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_winter_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_christmas_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_christmas_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// const seasonal_logo_binary_t logo_easter_binary\{ logobinary, sizeof\(logobinary\) \};', 'const seasonal_logo_binary_t logo_easter_binary{ logobinary, sizeof(logobinary) };'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Summer\);', 'load_menu_logo_texture(device, menu_season_t::Summer);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Spring\);', 'load_menu_logo_texture(device, menu_season_t::Spring);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Autumn\);', 'load_menu_logo_texture(device, menu_season_t::Autumn);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Winter\);', 'load_menu_logo_texture(device, menu_season_t::Winter);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Christmas\);', 'load_menu_logo_texture(device, menu_season_t::Christmas);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Easter\);', 'load_menu_logo_texture(device, menu_season_t::Easter);'
$content = $content -replace '// ped_hitbox_photo\.load\(device, pedhitbox, sizeof\(pedhitbox\)\);', 'ped_hitbox_photo.load(device, pedhitbox, sizeof(pedhitbox));'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Restored original logo code"
