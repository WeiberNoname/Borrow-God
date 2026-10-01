@echo off
if exist "%~dp0Builds\Windows\BorrowedGods.exe" (
  start "Borrowed Gods" "%~dp0Builds\Windows\BorrowedGods.exe" -windowed -ResX=1440 -ResY=900
  exit /b 0
)
set "BG_ENGINE=C:\Program Files\Epic Games\UE_5.7"
if not exist "%BG_ENGINE%\Engine\Binaries\Win64\UnrealEditor.exe" (
  echo Unreal Engine 5.7 was not found. Edit BG_ENGINE in this launcher.
  pause
  exit /b 1
)
start "Borrowed Gods" "%BG_ENGINE%\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0BorrowedGods.uproject" /Game/Maps/Archipelago -game -windowed -ResX=1440 -ResY=900 -NoTrace "-ShaderWorkingDir=%~dp0Intermediate\Shaders"
