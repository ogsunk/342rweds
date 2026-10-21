$imagesPath = 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
$newLogoPath = 'C:\Users\tyler\Desktop\Wizard External\logobinary_new.txt'

# Read the original file
$lines = Get-Content $imagesPath

# Find start and end of logobinary array
$start = 343258
$end = 373707

# Read new logo content
$newLogo = Get-Content $newLogoPath -Raw

# Create new file content
$newLines = $lines[0..($start-1)]
$newLines += $newLogo
$newLines += $lines[($end+1)..($lines.Count-1)]

# Write back
$newLines | Out-File -FilePath $imagesPath -Encoding ASCII

Write-Output "Logo replaced successfully"
