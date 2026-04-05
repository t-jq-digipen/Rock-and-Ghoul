@echo off
setlocal enabledelayedexpansion

set "REPO_ROOT=%~dp0"
if "!REPO_ROOT:~-1!"=="\" set "REPO_ROOT=!REPO_ROOT:~0,-1!"

set "BUILD_DIR=!REPO_ROOT!\build"
set "PROJECT_NAME=team_aviddivas"

REM Always open editor game picker
set "SELECTED_GAME="

echo.
echo ==========================================
echo   Select Build Configuration
echo ==========================================
echo   1) Debug
echo   2) Release
echo.
set /p "CONFIG_CHOICE=Enter choice (1/2): "

if "!CONFIG_CHOICE!"=="1" (
    set "CONFIG=Debug"
) else if "!CONFIG_CHOICE!"=="2" (
    set "CONFIG=Release"
) else (
    echo Invalid choice. Defaulting to Debug.
    set "CONFIG=Debug"
)

REM ==========================================
REM Configure (only if not yet configured)
REM ==========================================
if not exist "!BUILD_DIR!\CMakeCache.txt" (
    echo.
    echo Configuring project...
    echo ==========================================
    cmake -S "!REPO_ROOT!" -B "!BUILD_DIR!" -A x64
    if errorlevel 1 (
        echo.
        echo ERROR: CMake configure failed.
        pause
        exit /b 1
    )
)

REM ==========================================
REM Build
REM ==========================================
echo.
echo Building !CONFIG!...
echo ==========================================
cmake --build "!BUILD_DIR!" --config !CONFIG! --target !PROJECT_NAME!
if errorlevel 1 (
    echo.
    echo ERROR: Build failed.
    pause
    exit /b 1
)

REM ==========================================
REM Run
REM ==========================================
set "EXE_PATH=!BUILD_DIR!\!CONFIG!\!PROJECT_NAME!.exe"

if not exist "!EXE_PATH!" (
    echo.
    echo ERROR: Executable not found after build:
    echo   !EXE_PATH!
    pause
    exit /b 1
)

echo.
echo Launching editor [!CONFIG!]...
echo ==========================================
echo.

"!EXE_PATH!"

pause