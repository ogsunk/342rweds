# Delete the modified images.h and copy from backup if exists, or just rebuild with original logo
$backup = 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h.backup'
if (Test-Path $backup) {
    Copy-Item $backup 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
    Write-Output "Restored from backup"
} else {
    Write-Output "No backup found, keeping current file"
}
