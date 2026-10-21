$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw
# Uncomment the logo loading
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Summer\);', 'load_menu_logo_texture(device, menu_season_t::Summer);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Spring\);', 'load_menu_logo_texture(device, menu_season_t::Spring);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Autumn\);', 'load_menu_logo_texture(device, menu_season_t::Autumn);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Winter\);', 'load_menu_logo_texture(device, menu_season_t::Winter);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Christmas\);', 'load_menu_logo_texture(device, menu_season_t::Christmas);'
$content = $content -replace '// load_menu_logo_texture\(device, menu_season_t::Easter\);', 'load_menu_logo_texture(device, menu_season_t::Easter);'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Uncommented logo loading"
