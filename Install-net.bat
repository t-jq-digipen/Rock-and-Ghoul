@echo off
setlocal EnableExtensions EnableDelayedExpansion
title .NET Framework 4.7.2 Installer (Auto Detect + Install)

echo ==========================================================
echo       .NET Framework 4.7.2 Developer Pack Installer
echo ==========================================================
echo.

rem --- Step 0: Elevation check (need admin for install) ---
>nul 2>&1 net session
if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Please run this script as Administrator.
  pause
  exit /b 1
)

rem --- Step 1: Detect existing .NET 4.7.2+ ---
echo [STEP 1] Checking if .NET 4.7.2 or later is installed...

set "NETKEY=HKLM\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full"
reg query "%NETKEY%" /v Release >nul 2>&1

if %ERRORLEVEL% NEQ 0 (
  rem 32-bit redirection fallback on some systems
  set "NETKEY=HKLM\SOFTWARE\WOW6432Node\Microsoft\NET Framework Setup\NDP\v4\Full"
  reg query "%NETKEY%" /v Release >nul 2>&1
)

if %ERRORLEVEL% NEQ 0 (
  echo [INFO] No .NET 4.x Full key found.
) else (
  for /f "tokens=3" %%v in ('reg query "%NETKEY%" /v Release ^| find "Release"') do set "version=%%v"
  echo [DEBUG] Found Release key: !version!
  rem 4.7.2 Release key is 461808 (or 461814 on some OS builds)
  if !version! GEQ 461808 (
    echo [OK] .NET Framework 4.7.2 or newer is already installed.
    pause
    exit /b 0
  )
)

echo.
echo [STEP 2] .NET 4.7.2 not found — downloading installer...
set "DOTNET_URL=https://download.visualstudio.microsoft.com/download/pr/1a7f7f2e-8b58-4c54-bf3b-316f1b52b4df/ca1242dcd4974bb0c2fbc799f61b5d60/ndp472-devpack-enu.exe"
set "DOTNET_FILE=%TEMP%\ndp472-devpack-enu.exe"

rem Try WebClient first, then BITS as fallback
powershell -NoLogo -NoProfile -Command ^
  "Try { (New-Object Net.WebClient).DownloadFile('%DOTNET_URL%','%DOTNET_FILE%') } Catch { exit 1 }"
if not exist "%DOTNET_FILE%" (
  echo [WARN] WebClient failed; trying BITS...
  powershell -NoLogo -NoProfile -Command ^
    "Try { Start-BitsTransfer -Source '%DOTNET_URL%' -Destination '%DOTNET_FILE%' } Catch { exit 1 }"
)

if not exist "%DOTNET_FILE%" (
  echo [ERROR] Failed to download .NET 4.7.2 Dev Pack.
  pause
  exit /b 1
)
echo [OK] Downloaded: %DOTNET_FILE%

echo.
echo [STEP 3] Installing .NET Framework 4.7.2 Developer Pack silently...
"%DOTNET_FILE%" /quiet /norestart
set "INSTALL_RC=%ERRORLEVEL%"

echo [INFO] Installer exit code: !INSTALL_RC!
if not "!INSTALL_RC!"=="0" (
  echo [WARN] Installer returned a non-zero code. Continuing to verify install...
)

rem Some systems update the key a few seconds after the process ends
echo Waiting for registry to update...
for /l %%i in (1,1,10) do (
  timeout /t 2 >nul
  reg query "%NETKEY%" /v Release >nul 2>&1
  if !ERRORLEVEL! EQU 0 (
    for /f "tokens=3" %%v in ('reg query "%NETKEY%" /v Release ^| find "Release"') do set "version=%%v"
    if defined version if !version! GEQ 461808 goto :installed_ok
  )
)

echo [ERROR] .NET installation failed or is incomplete.
pause
exit /b 1

:installed_ok
echo [OK] .NET Framework 4.7.2 installed successfully! (Release=!version!)
pause
exit /b 0
