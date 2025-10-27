@echo off
echo ==========================================================
echo       Mono Runtime Installer + Auto Detection + Copy
echo ==========================================================
echo.

REM === 1. set download info ===
set MONO_URL=https://download.mono-project.com/archive/6.12.0/windows-installer/mono-6.12.0.206-x64-0.msi
set MONO_FILE=%TEMP%\mono-installer.msi

REM repo paths
set REPO_DIR=%~dp0
set REPO_MONO_DIR=%REPO_DIR%deps\Mono

echo This script will:
echo  - Download Mono (6.12.0) for Windows
echo  - Install it silently if needed
echo  - Auto-detect install path (x64 or x86)
echo  - Copy into local deps\Mono folder
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

REM wait a bit for installation to finish
timeout /t 5 >nul

REM === 4. auto-detect install location ===
set MONO_INSTALL_PATH=

REM check typical 64-bit location
if exist "C:\Program Files\Mono\bin\mono.exe" (
    set MONO_INSTALL_PATH=C:\Program Files\Mono
)

REM fallback to 32-bit location
if exist "C:\Program Files (x86)\Mono\bin\mono.exe" (
    set MONO_INSTALL_PATH=C:\Program Files (x86)\Mono
)

REM === 5. handle detection failure ===
if "%MONO_INSTALL_PATH%"=="" (
    echo ❌ Could not detect Mono installation automatically.
    echo Searching manually...
    for /d %%D in ("C:\Program Files*\Mono") do (
        if exist "%%D\bin\mono.exe" (
            set MONO_INSTALL_PATH=%%D
        )
    )
)

REM final check
if "%MONO_INSTALL_PATH%"=="" (
    echo ❌ Mono installation not found anywhere under Program Files.
    echo Please install manually from https://www.mono-project.com/download/stable/#download-win
    pause
    exit /b 1
)

echo ✅ Mono detected at:
echo   %MONO_INSTALL_PATH%
echo.

REM === 6. create deps\Mono folder ===
if not exist "%REPO_DIR%deps" mkdir "%REPO_DIR%deps"
if not exist "%REPO_MONO_DIR%" mkdir "%REPO_MONO_DIR%"
echo Ensured deps\Mono folder exists.

REM === 7. copy contents ===
echo Copying Mono SDK into your local repo...
xcopy "%MONO_INSTALL_PATH%\*" "%REPO_MONO_DIR%\" /E /I /Y >nul

if %ERRORLEVEL%==0 (
    echo ✅ Copied Mono successfully to:
    echo   %REPO_MONO_DIR%
) else (
    echo ⚠️  Copy operation completed with some warnings.
)

REM === 8. verify runtime DLL ===
if exist "%REPO_MONO_DIR%\bin\mono-2.0-sgen.dll" (
    echo ✅ Mono runtime DLL found in deps\Mono\bin\
) else (
    echo ⚠️  Mono runtime DLL not found after copy.
)

echo.
echo ==========================================================
echo     Installation + Copy completed successfully!
echo     You can now build your CMake project.
echo ==========================================================
echo.
pause
