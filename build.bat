@echo off
REM Build script for Vanguard Emulator

echo ========================================
echo   Vanguard Emulator Build Script
echo ========================================
echo.

if "%1"=="clean" (
    echo Cleaning build artifacts...
    if exist bin rmdir /s /q bin
    if exist obj rmdir /s /q obj
    if exist .vs rmdir /s /q .vs
    echo Clean complete.
    goto :eof
)

if not exist "C:\Program Files\Microsoft Visual Studio" (
    echo ERROR: Visual Studio not found
    echo Please install Visual Studio 2019 or later
    exit /b 1
)

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

echo Building configuration: %CONFIG%
echo.

msbuild VanguardEmulator.sln /p:Configuration=%CONFIG% /p:Platform=x64 /m

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   Build Successful!
    echo ========================================
    echo.
    echo Output: bin\%CONFIG%\VanguardEmulator.exe
    echo.
    echo Run with Administrator privileges:
    echo   Right-click VanguardEmulator.exe ^> Run as administrator
) else (
    echo.
    echo ========================================
    echo   Build Failed!
    echo ========================================
    exit /b 1
)
