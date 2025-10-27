@echo off
setlocal enabledelayedexpansion
title Mono Installer (Debug Mode)
echo ==========================================================
echo         DEBUG MODE - Mono Installer + Copy
echo ==========================================================
echo.

REM 1. download info
set MONO_URL=https://download.mono-project.com/archive/6.12.0/windows-installer/mono-6.12.0.206-x64-0.msi
set MONO_FILE=%TEMP%\mono-installer.msi
set REPO_DIR=%~dp0
set REPO_MONO_DIR=%REPO_DIR%deps\Mono

echo Repo path: %REPO_DIR%
echo Target deps\Mono folder: %REPO_MONO_DIR%
echo.
pause

echo [STEP 1] Downloading Mono installer...
powershell -Command "(New-Object Net.WebClient).DownloadFile('%MONO_URL%', '%MONO_FILE%')"
if not exist "%MONO_FILE%" (
    echo ❌ Failed to download Mono installer.
    pause
    exit /b 1
)
echo ✅ Download complete: %MONO_FILE%
pause

echo [STEP 2] Installing Mono silently...
msiexec /i "%MONO_FILE%" /quiet /norestart
if errorlevel 1 (
    echo ⚠️  msiexec returned error code %errorlevel%
)
timeout /t 5 >nul
echo Installation attempt complete.
pause

echo [STEP 3] Searching for Mono installation...
set MONO_INSTALL_PATH=
for /d %%D in ("C:\Program Files*\Mono") do (
    if exist "%%D\bin\mono.exe" (
        set "MONO_INSTALL_PATH=%%D"
    )
)
if "%MONO_INSTALL_PATH%"=="" (
    echo ❌ Could not find mono.exe under Program Files.
) else (
    echo ✅ Found Mono at: !MONO_INSTALL_PATH!
)
pause

echo [STEP 4] Creating deps\Mono folder if missing...
if not exist "%REPO_DIR%deps" mkdir "%REPO_DIR%deps"
if not exist "%REPO_MONO_DIR%" mkdir "%REPO_MONO_DIR%"
echo Folder ready: %REPO_MONO_DIR%
pause

echo [STEP 5] Copying Mono files...
if not "%MONO_INSTALL_PATH%"=="" (
    xcopy "%MONO_INSTALL_PATH%\*" "%REPO_MONO_DIR%\" /E /I /Y
    echo ✅ Copy complete.
) else (
    echo ⚠️  Skipped copy — install path not found.
)
pause

echo [STEP 6] Verifying copy...
if exist "%REPO_MONO_DIR%\bin\mono-2.0-sgen.dll" (
    echo ✅ DLL found: deps\Mono\bin\mono-2.0-sgen.dll
) else (
    echo ❌ DLL not found.
)
echo.
echo ==========================================================
echo        Debug completed — check output above
echo ==========================================================
pause
