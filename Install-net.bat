@echo off
setlocal EnableExtensions EnableDelayedExpansion
title .NET Framework 4.7.2 Developer Pack Auto Installer

echo ==========================================================
echo     .NET Framework 4.7.2 Developer Pack Installer
echo ==========================================================
echo.

rem ───────────────────────────────────────────────
rem STEP 0: Check for admin
rem ───────────────────────────────────────────────
>nul 2>&1 net session
if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Please right-click this file and choose:
  echo          "Run as administrator"
  echo.
  pause
  exit /b 1
)

rem ───────────────────────────────────────────────
rem STEP 1: Check if .NET 4.7.2+ runtime is installed
rem ───────────────────────────────────────────────
set "NETKEY=HKLM\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full"
reg query "%NETKEY%" /v Release >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
  set "NETKEY=HKLM\SOFTWARE\WOW6432Node\Microsoft\NET Framework Setup\NDP\v4\Full"
  reg query "%NETKEY%" /v Release >nul 2>&1
)

set "foundVer="
for /f "tokens=3" %%v in ('reg query "%NETKEY%" /v Release 2^>nul ^| find "Release"') do set "foundVer=%%v"

if defined foundVer (
  if !foundVer! GEQ 461808 (
    echo [OK] .NET Framework runtime 4.7.2 or newer is installed. (Release=!foundVer!)
  ) else (
    echo [WARN] Detected older .NET version (Release=!foundVer!). Needs update.
  )
) else (
  echo [INFO] .NET 4.x runtime not found.
)

rem ───────────────────────────────────────────────
rem STEP 2: Check if Developer Pack (Targeting Pack) is installed
rem ───────────────────────────────────────────────
set "DEVKEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\NETFXSDK\4.7.2\WinSDK-NetFx40Tools-x86"
reg query "%DEVKEY%" /v InstallationFolder >nul 2>&1
if %ERRORLEVEL% EQU 0 (
  for /f "tokens=3*" %%a in ('reg query "%DEVKEY%" /v InstallationFolder 2^>nul ^| find "InstallationFolder"') do set "devPack=%%b"
)

if defined devPack (
  echo [OK] .NET 4.7.2 Developer Pack already installed at:
  echo      !devPack!
  echo.
  pause
  exit /b 0
)

rem ───────────────────────────────────────────────
rem STEP 3: Download Developer Pack
rem ───────────────────────────────────────────────
echo [STEP 3] Downloading .NET Framework 4.7.2 Developer Pack...
set "DOTNET_URL=https://download.visualstudio.microsoft.com/download/pr/1a7f7f2e-8b58-4c54-bf3b-316f1b52b4df/ca1242dcd4974bb0c2fbc799f61b5d60/ndp472-devpack-enu.exe"
set "DOTNET_FILE=%TEMP%\ndp472-devpack-enu.exe"

if exist "%DOTNET_FILE%" del "%DOTNET_FILE%" >nul 2>&1

powershell -NoLogo -NoProfile -Command ^
  "Try { (New-Object Net.WebClient).DownloadFile('%DOTNET_URL%','%DOTNET_FILE%') } Catch { exit 1 }"

if not exist "%DOTNET_FILE%" (
  echo [ERROR] Failed to download .NET 4.7.2 Developer Pack.
  echo.
  pause
  exit /b 1
)
echo [OK] Downloaded to: %DOTNET_FILE%
echo.

rem ───────────────────────────────────────────────
rem STEP 4: Install silently
rem ───────────────────────────────────────────────
echo [STEP 4] Installing .NET 4.7.2 Developer Pack (please wait)...
"%DOTNET_FILE%" /quiet /norestart
set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
  echo [WARN] Silent install may have failed (exit code %RC%).
  echo [INFO] Retrying with visible installer...
  "%DOTNET_FILE%"
)

rem ───────────────────────────────────────────────
rem STEP 5: Verify install
rem ───────────────────────────────────────────────
timeout /t 5 >nul
reg query "%DEVKEY%" /v InstallationFolder >nul 2>&1
if %ERRORLEVEL% EQU 0 (
  echo [OK] .NET 4.7.2 Developer Pack installation verified successfully!
) else (
  echo [ERROR] Developer Pack installation failed or not detected.
  echo [INFO] Try running this script again, or install manually from:
  echo        https://dotnet.microsoft.com/en-us/download/dotnet-framework/net472
)

echo.
pause
exit /b 0
