param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7', [switch]$Packaged)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$profileRoot = Join-Path $projectRoot 'Saved\ArtProfile'
$previewArguments = @('-BGVisualPreview', '-unattended', '-RenderOffscreen', '-windowed', '-ForceRes', '-ResX=1440', '-ResY=900', '-NoSound', '-NoTrace', "`"-UserDir=$profileRoot`"", "`"-abslog=$projectRoot\Saved\Logs\ArtPreview.log`"")
if ($Packaged) {
    $previewExe = "$projectRoot\Builds\Windows\BorrowedGods\Binaries\Win64\BorrowedGods.exe"
    $previewWorkingDirectory = "$projectRoot\Builds\Windows"
} else {
    $previewExe = "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    $previewWorkingDirectory = $projectRoot
    $previewArguments = @("`"$projectRoot\BorrowedGods.uproject`"", '/Game/Maps/Archipelago', '-game', "`"-ShaderWorkingDir=$projectRoot\Intermediate\Shaders`"") + $previewArguments
}
$previewProcess = Start-Process -FilePath $previewExe -ArgumentList $previewArguments -WorkingDirectory $previewWorkingDirectory -WindowStyle Hidden -PassThru
$previewProcess.WaitForExit()
if ($previewProcess.ExitCode -ne 0) { throw "Art preview failed: $($previewProcess.ExitCode)" }
Write-Output "Art captures: $profileRoot\Saved\Screenshots"
