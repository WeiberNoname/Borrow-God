param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7', [switch]$Packaged)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if ($Packaged) {
    $profileRoot = Join-Path $projectRoot 'Saved\PackagedProfile'
    $resultPath = Join-Path $profileRoot 'Saved\SmokeTestResult.txt'
    if (Test-Path -LiteralPath $resultPath) { Remove-Item -LiteralPath $resultPath }
    $testArguments = @('-BGSmokeTest', '-unattended', '-RenderOffscreen', '-windowed', '-ForceRes', '-ResX=1280', '-ResY=720', '-NoSound', '-NoTrace', "`"-UserDir=$profileRoot`"", "`"-abslog=$projectRoot\Saved\Logs\PackagedSmoke.log`"")
    $testProcess = Start-Process -FilePath "$projectRoot\Builds\Windows\BorrowedGods\Binaries\Win64\BorrowedGods.exe" -ArgumentList $testArguments -WorkingDirectory "$projectRoot\Builds\Windows" -WindowStyle Hidden -PassThru
    $testProcess.WaitForExit()
    $testExitCode = $testProcess.ExitCode
} else {
    $resultPath = Join-Path $projectRoot 'Saved\SmokeTestResult.txt'
    if (Test-Path -LiteralPath $resultPath) { Remove-Item -LiteralPath $resultPath }
    & "$EngineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$projectRoot\BorrowedGods.uproject" /Game/Maps/Archipelago -game -BGSmokeTest -unattended -RenderOffscreen -windowed -ForceRes -ResX=1440 -ResY=900 -NoSound -NoTrace -log "-ShaderWorkingDir=$projectRoot\Intermediate\Shaders"
    $testExitCode = $LASTEXITCODE
}
if ($testExitCode -ne 0) { throw "Smoke test failed: $testExitCode" }
$result = Get-Content -LiteralPath $resultPath
if ($result.Trim() -ne 'PASS') { throw 'Smoke test did not pass.' }
Write-Output 'Borrowed Gods integration smoke test passed.'
