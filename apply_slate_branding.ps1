$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Raw

# Apply namespace changes in slate.cpp
$content = $content -replace 'Core::Features::WizardVisuals::IsEnabled', 'Core::Features::SunketsWareVisuals::IsEnabled'
$content = $content -replace 'Core::Features::WizardVisuals::MatchesName', 'Core::Features::SunketsWareVisuals::MatchesName'
$content = $content -replace 'Core::Features::WizardVisuals::SetEnabled', 'Core::Features::SunketsWareVisuals::SetEnabled'
$content = $content -replace 'Core::Features::WizardVisuals::SetLogoTexture', 'Core::Features::SunketsWareVisuals::SetLogoTexture'
$content = $content -replace 'Core::Features::WizardVisuals::SetNames', 'Core::Features::SunketsWareVisuals::SetNames'

$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\src\slate.cpp' -Encoding ASCII
Write-Output "Applied slate.cpp branding"
