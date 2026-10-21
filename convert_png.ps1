$bytes = [System.IO.File]::ReadAllBytes('C:\Users\tyler\Desktop\Wizard External\src\slate\resources\sunketsware.png')
Write-Output "unsigned char sunketsware[$($bytes.Length)] = {"
$output = ($bytes | ForEach-Object { "0x{0:X2}" -f $_ }) -join ', '
Write-Output $output
Write-Output "};"
