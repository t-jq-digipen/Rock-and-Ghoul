@echo off
setlocal enabledelayedexpansion
title Mono Installer + Forced Path + Auto Copy
echo ==========================================================
echo         Mono Installer - Forced Path + Copy
echo ==========================================================
echo
echo This will:
echo   1. Install .NET Framework 4.7.2 Developer Pack (if missing)
echo   2. Install Mono 6.12.0 to "C:\Program Files\Mono"
echo   3. Copy Mono into your local deps\Mono folder
echo ==========================================================
echo.
pause

REM === 0: Run .NET Framework installer ===
echo [STEP 1] Running install-net.bat ...
if exist "%~dp0install-net.bat" (
    call "%~dp0install-net.bat"
    if %ERRORLEVEL% NEQ 0 (
        echo install-net.bat failed or was cancelled.
        pause
        exit /b 1
    )
) else (
    echo install-net.bat not found in current directory.
    pause
)
REM === 1. Setup paths and variables ===
set MONO_URL=https://download.mono-project.com/archive/6.12.0/windows-installer/mono-6.12.0.206-x64-0.msi
set MONO_FILE=%TEMP%\mono-installer.msi
set MONO_INSTALL_PATH=C:\Program Files\Mono
set REPO_DIR=%~dp0
set REPO_MONO_DIR=%REPO_DIR%deps\Mono

echo This will:
echo   1. Download Mono 6.12.0 for Windows
echo   2. Force install it to "%MONO_INSTALL_PATH%"
echo   3. Copy to "%REPO_MONO_DIR%"
echo.
pause

REM === 2. Check if Mono already exists in deps ===
if exist "%REPO_MONO_DIR%\bin\mono.exe" (
    echo Existing Mono installation found in deps\Mono.
    echo Skipping download, installation, and copy steps.
    echo.
    pause
    exit /b 0
)

REM === 2. Download Mono MSI ===
echo Downloading Mono installer...
powershell -Command "(New-Object Net.WebClient).DownloadFile('%MONO_URL%', '%MONO_FILE%')"

if not exist "%MONO_FILE%" (
    echo Failed to download Mono installer.
    pause
    exit /b 1
)

REM === 3. Install Mono (forced directory) ===
echo Installing Mono silently to "%MONO_INSTALL_PATH%"...
msiexec /i "%MONO_FILE%" TARGETDIR="%MONO_INSTALL_PATH%" /quiet /norestart

echo Waiting for installer to complete...
timeout /t 5 >nul

REM === 4. Verify installation ===
if not exist "%MONO_INSTALL_PATH%\bin\mono.exe" (
    echo Mono not found at "%MONO_INSTALL_PATH%".
    echo Installation might have failed.
    pause
    exit /b 1
)
echo Mono installed at: %MONO_INSTALL_PATH%
pause

REM === 5. Create deps\Mono folder ===
if not exist "%REPO_DIR%deps" mkdir "%REPO_DIR%deps"
if not exist "%REPO_MONO_DIR%" mkdir "%REPO_MONO_DIR%"
echo Created deps\Mono folder if missing.

REM === 6. Copy all Mono files into repo ===
echo Copying Mono files to local deps\Mono...
xcopy "%MONO_INSTALL_PATH%\*" "%REPO_MONO_DIR%\" /E /I /Y >nul

if %ERRORLEVEL%==0 (
    echo Copied Mono to %REPO_MONO_DIR%
) else (
    echo Copy operation encountered warnings.
)

REM === 7. Verify the runtime DLL ===
if exist "%REPO_MONO_DIR%\bin\mono-2.0-sgen.dll" (
    echo Mono runtime DLL verified in deps\Mono\bin\
) else (
    echo mono-2.0-sgen.dll missing — please recheck copy.
)

echo.
echo ==========================================================
echo        Installation and Copy completed successfully!
echo ==========================================================
echo.
pause
