$bytes = [System.IO.File]::ReadAllBytes('C:\Users\tyler\Desktop\Wizard External\src\slate\resources\sunketsware.png')
Write-Output "inline unsigned char logobinary[] = {"
for($i = 0; $i -lt $bytes.Count; $i += 16) {
    $chunk = $bytes[$i..[Math]::Min($i+15, $bytes.Count-1)]
    $hex = ($chunk | ForEach-Object { "0x{0:X2}" -f $_ }) -join ', '
    Write-Output "	$hex"
}
Write-Output "};"
