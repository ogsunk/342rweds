$content = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Raw
if ($content -notmatch 'pedhitbox') {
    $content = $content -replace 'inline unsigned char logobinary\[4286\] = \{', 'inline unsigned char logobinary[4286] = {`n`ninline unsigned char pedhitbox[] = {`n};'
    $content | Out-File -FilePath 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h' -Encoding ASCII
    Write-Output "Added pedhitbox placeholder"
} else {
    Write-Output "pedhitbox already exists"
}
