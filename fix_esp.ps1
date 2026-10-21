$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\Core\Features\Esp.cpp' -Raw
$content = $content -replace 'wizardLogoTexture', 'sunketswareLogoTexture'
$content = $content -replace 'showWizardLogo', 'showSunketsWareLogo'
$content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\Core\Features\Esp.cpp' -Encoding ASCII
Write-Output "Fixed Esp.cpp variable names"
