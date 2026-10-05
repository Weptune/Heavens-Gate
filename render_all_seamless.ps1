# PowerShell script to render all 12 seamless scenes with frame-to-frame continuity
$scenes = @(
    @{ File = "animations/scene01_the_network.py"; Class = "Scene01TheNetwork"; Output = "Scene01TheNetwork.mp4" },
    @{ File = "animations/scene02_what_makes_it_spectral.py"; Class = "Scene02WhatMakesItSpectral"; Output = "Scene02WhatMakesItSpectral.mp4" },
    @{ File = "animations/scene03_the_fiedler_breakthrough.py"; Class = "Scene03TheFiedlerBreakthrough"; Output = "Scene03TheFiedlerBreakthrough.mp4" },
    @{ File = "animations/scene04_the_black_box_era.py"; Class = "Scene04TheBlackBoxEra"; Output = "Scene04TheBlackBoxEra.mp4" },
    @{ File = "animations/scene05_the_paradigm_shift.py"; Class = "Scene05TheParadigmShift"; Output = "Scene05TheParadigmShift.mp4" },
    @{ File = "animations/scene06_the_complexity_trap.py"; Class = "Scene06ComplexityTrap"; Output = "Scene06ComplexityTrap.mp4" },
    @{ File = "animations/scene07_tropical_odyssey.py"; Class = "Scene07TropicalOdyssey"; Output = "Scene07TropicalOdyssey.mp4" },
    @{ File = "animations/scene08_hall_of_shame.py"; Class = "Scene08HallOfShame"; Output = "Scene08HallOfShame.mp4" },
    @{ File = "animations/scene09_the_split_brain.py"; Class = "Scene09TheSplitBrain"; Output = "Scene09TheSplitBrain.mp4" },
    @{ File = "animations/scene10_the_classical_sovereign.py"; Class = "Scene10TheClassicalSovereign"; Output = "Scene10TheClassicalSovereign.mp4" },
    @{ File = "animations/scene11_the_stockfish_gauntlet.py"; Class = "Scene11TheStockfishGauntlet"; Output = "Scene11TheStockfishGauntlet.mp4" },
    @{ File = "animations/scene12_the_sovereign_arena.py"; Class = "Scene12TheSovereignArena"; Output = "Scene12TheSovereignArena.mp4" }
)

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "RENDERING ALL 12 SEAMLESS CONTINUOUS SCENES" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan

foreach ($s in $scenes) {
    Write-Host "[RENDERING] $($s.Class) from $($s.File)..." -ForegroundColor Yellow
    python -m manimlib $s.File $s.Class -w --hd --file_name $s.Output --video_dir videos
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Failed to render $($s.Class)!" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Write-Host "[SUCCESS] Rendered $($s.Output)" -ForegroundColor Green
}

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "ALL 12 SCENES RENDERED CLEANLY" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Cyan
