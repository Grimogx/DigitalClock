@echo off
setlocal
cd /d "%~dp0"

where cl >nul 2>nul
if errorlevel 1 (
    echo Microsoft C compiler not found. Run this from a Visual Studio Developer Command Prompt.
    exit /b 1
)

if not exist build mkdir build

cl /nologo /std:c11 /W4 /O2 /MT /Isrc ^
    src\main.c src\clock.c src\stopwatch.c src\settings.c src\gui.c ^
    /Fe:build\DigitalClock.exe ^
    /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib

if errorlevel 1 exit /b 1
echo Built build\DigitalClock.exe
