@echo off
setlocal enabledelayedexpansion

set "REPO_ROOT=%~dp0"
if "!REPO_ROOT:~-1!"=="\" set "REPO_ROOT=!REPO_ROOT:~0,-1!"

set "BUILD_DIR=!REPO_ROOT!\build"

echo ==========================================
echo   Cleaning build output...
echo ==========================================

if exist "!BUILD_DIR!" (
    echo   Removing build\...
    rmdir /s /q "!BUILD_DIR!"
    echo   Done.
) else (
    echo   Nothing to clean.
)

echo.
pause
