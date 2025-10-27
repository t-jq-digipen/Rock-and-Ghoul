@echo off
echo ==========================================================
echo        Mono Runtime Installer + Auto Repo Copier
echo ==========================================================
echo.

REM === 1. set variables ===
set MONO_URL=https://download.mono-project.com/archive/6.12.0/windows-installer/mono-6.12.0.206-x64-0.msi
set MONO_FILE=%TEMP%\mono-installer.msi
set MONO_INSTALL_PATH=C:\Program Files\Mono
set REPO_DIR=%~dp0
set REPO_MONO_DIR=%REPO_DIR%deps\Mono

echo This script will:
echo  - Download Mono from: %MONO_URL%
echo  - Install it silently to: %MONO_INSTALL_PATH%
echo  - Copy into local project folder: %REPO_MONO_DIR%
echo.
pause

REM === 2. download installer ===
echo Downloading Mono installer...
powershell -Command "(New-Object Net.WebClient).DownloadFile('%MONO_URL%', '%MONO_FILE%')"

if not exist "%MONO_FILE%" (
    echo ❌ Failed to download Mono installer.
    pause
    exit /b 1
)

REM === 3. install silently ===
echo Installing Mono silently (please wait)...
msiexec /i "%MONO_FILE%" /quiet /norestart

REM wait for installation to finish
timeout /t 5 >nul

REM === 4. verify installation ===
if not exist "%MONO_INSTALL_PATH%\bin\mono.exe" (
    echo ❌ Mono installation not found at %MONO_INSTALL_PATH%
    echo Please check if installation completed properly.
    pause
    exit /b 1
)

echo ✅ Mono installed successfully at:
echo   %MONO_INSTALL_PATH%
echo.

REM === 5. create deps\Mono folder if missing ===
if not exist "%REPO_DIR%deps" mkdir "%REPO_DIR%deps"
if not exist "%REPO_MONO_DIR%" mkdir "%REPO_MONO_DIR%"
echo Created deps\Mono folder (if missing).

REM === 6. copy contents ===
echo Copying Mono SDK into your local repo...
xcopy "%MONO_INSTALL_PATH%\*" "%REPO_MONO_DIR%\" /E /I /Y >nul

if %ERRORLEVEL%==0 (
    echo ✅ Copied Mono successfully to:
    echo   %REPO_MONO_DIR%
) else (
    echo ⚠️  Copy operation completed with some warnings.
)

REM === 7. final check ===
if exist "%REPO_MONO_DIR%\bin\mono-2.0-sgen.dll" (
    echo ✅ Mono runtime DLL found in deps\Mono\bin\
) else (
    echo ⚠️  Mono runtime DLL missing — please check copy.
)

echo.
echo ==========================================================
echo     Installation complete!
echo     You can now build your project in Visual Studio.
echo ==========================================================
echo.
pause
