$lines = Get-Content 'C:\Users\tyler\Desktop\Wizard External\src\slate\resources\images.h'
$start = 343258
$inArray = $false
$end = 0
for($i = $start; $i -lt $lines.Count; $i++) {
    if($lines[$i] -match '\{$') { $inArray = $true }
    if($inArray -and $lines[$i] -match '\};') { $end = $i; break }
}
Write-Output "Start: $start, End: $end"
