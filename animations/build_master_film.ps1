# build_master_film.ps1
# 1. Renders all 12 seamless scenes in sequence
# 2. Concatentates all 12 MP4s into a single master continuous broadcast film

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "STEP 1: RENDERING ALL 12 SEAMLESS CONTINUOUS SCENES" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan

& "$PSScriptRoot\render_all_seamless.ps1"
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Scene rendering failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "STEP 2: CONCATENATING INTO MASTER FILM VIA FFMPEG" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan

$concatContent = @"
file 'videos/Scene01TheNetwork.mp4'
file 'videos/Scene02WhatMakesItSpectral.mp4'
file 'videos/Scene03TheFiedlerBreakthrough.mp4'
file 'videos/Scene04TheBlackBoxEra.mp4'
file 'videos/Scene05TheParadigmShift.mp4'
file 'videos/Scene06ComplexityTrap.mp4'
file 'videos/Scene07TropicalOdyssey.mp4'
file 'videos/Scene08HallOfShame.mp4'
file 'videos/Scene09TheSplitBrain.mp4'
file 'videos/Scene10TheClassicalSovereign.mp4'
file 'videos/Scene11TheStockfishGauntlet.mp4'
file 'videos/Scene12TheSovereignArena.mp4'
"@

Set-Content -Path "concat_list.txt" -Value $concatContent

ffmpeg -f concat -safe 0 -i "concat_list.txt" -c copy "videos/HeavensGate_TheDocumentary_Master.mp4" -y
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] FFmpeg concatenation failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "====================================================" -ForegroundColor Green
Write-Host "MASTER FILM READY: videos/HeavensGate_TheDocumentary_Master.mp4" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green

Get-Item "videos/HeavensGate_TheDocumentary_Master.mp4" | Select-Object Name, Length, LastWriteTime
