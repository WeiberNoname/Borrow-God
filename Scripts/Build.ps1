param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7', [ValidateSet('Editor','Game')][string]$Target = 'Editor')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$targetName = if ($Target -eq 'Game') { 'BorrowedGods' } else { 'BorrowedGodsEditor' }
& "$EngineRoot\Engine\Build\BatchFiles\Build.bat" $targetName Win64 Development "-Project=$projectRoot\BorrowedGods.uproject" "-Log=$projectRoot\Saved\Build-$Target.log" -WaitMutex -NoHotReloadFromIDE -NoUBA -NoUBALocal -NoUBTMakefiles
if ($LASTEXITCODE -ne 0) { throw "Unreal build failed with exit code $LASTEXITCODE" }
