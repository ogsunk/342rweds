$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw

# Apply only text branding changes - no logo modifications
$content = $content -replace 'Core/features/WizardVisuals.hpp', 'Core/features/SunketsWareVisuals.hpp'
$content = $content -replace 'Core::Features::WizardVisuals::SetLogoTexture', 'Core::Features::SunketsWareVisuals::SetLogoTexture'
$content = $content -replace 'Wizard fallback', 'SunketsWare fallback'
$content = $content -replace 'is_wizard', 'is_sunketsware'
$content = $content -replace 'request_wizard_names_refresh', 'request_sunketsware_names_refresh'
$content = $content -replace 'LoadWizardIngameNames', 'LoadSunketsWareIngameNames'
$content = $content -replace 'Core::Features::WizardVisuals::SetNames', 'Core::Features::SunketsWareVisuals::SetNames'
$content = $content -replace 'wizard_visuals_enabled', 'sunketsware_visuals_enabled'
$content = $content -replace 'wizard_names_state', 'sunketsware_names_state'
$content = $content -replace '"Wizard"', '"SunketsWare"'
$content = $content -replace '"WIZARD"', '"SUNKETSWARE"'

$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Applied text branding"
