param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7')
$projectRoot = Split-Path -Parent $PSScriptRoot
$packagedGame = Join-Path $projectRoot 'Builds\Windows\BorrowedGods.exe'
if (Test-Path -LiteralPath $packagedGame) {
    Start-Process -FilePath $packagedGame -ArgumentList @('-windowed', '-ResX=1440', '-ResY=900') -WorkingDirectory (Split-Path -Parent $packagedGame)
    return
}
Start-Process -FilePath "$EngineRoot\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList @("`"$projectRoot\BorrowedGods.uproject`"", '/Game/Maps/Archipelago', '-game', '-windowed', '-ResX=1440', '-ResY=900', "`"-ShaderWorkingDir=$projectRoot\Intermediate\Shaders`"") -WorkingDirectory $projectRoot
