param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7', [switch]$SkipBuild, [switch]$SkipCook)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$sdkBins = Get-ChildItem -LiteralPath 'C:\Program Files (x86)\Windows Kits\10\bin' -Directory | Sort-Object Name -Descending
foreach ($sdkBin in $sdkBins) {
    $manifestTool = Join-Path $sdkBin.FullName 'x64\mt.exe'
    if (Test-Path -LiteralPath $manifestTool) { $env:PATH = "$(Split-Path -Parent $manifestTool);$env:PATH"; break }
}
$env:uebp_LogFolder = "$projectRoot\Saved\PackagingLogs"
$env:uebp_EngineSavedFolder = "$projectRoot\Saved\Automation"
$buildOption = if ($SkipBuild) { '-skipbuild' } else { '-build' }
$cookOption = if ($SkipCook) { '-skipcook' } else { '-cook' }
& "$EngineRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=$projectRoot\BorrowedGods.uproject" -noP4 -platform=Win64 -clientconfig=Development $buildOption $cookOption -stage -pak -archive "-archivedirectory=$projectRoot\Builds" '-UbtArgs=-NoUBA -NoUBALocal -NoUBTMakefiles' "-AdditionalCookerOptions=-ShaderWorkingDir=$projectRoot\Intermediate\CookShaders" -unattended -utf8output -nocompile -nocompileuat
if ($LASTEXITCODE -ne 0) { throw "Packaging failed: $LASTEXITCODE" }
Write-Output "Packaged game: $projectRoot\Builds\Windows\BorrowedGods.exe"
