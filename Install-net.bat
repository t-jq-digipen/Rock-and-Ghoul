@echo off
setlocal EnableExtensions EnableDelayedExpansion
title .NET Framework 4.8 Installer (Auto Detect + Install)

echo ==========================================================
echo       .NET Framework 4.8 Developer Pack Installer
echo ==========================================================
echo.

rem --- Step 0: Elevation check (need admin for install) ---
>nul 2>&1 net session
if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Please run this script as Administrator.
  pause
  exit /b 1
)

rem --- Step 1: Detect existing .NET 4.8+ ---
echo [STEP 1] Checking if .NET 4.8 or later is installed...

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
  rem 4.8 Release key is 528040 (or 528049 on Win10 1809+)
  if !version! GEQ 528040 (
    echo [OK] .NET Framework 4.8 or newer is already installed.
    pause
    exit /b 0
  )
)

echo.
echo [STEP 2] .NET 4.8 not found — downloading installer...
set "DOTNET_URL=https://download.visualstudio.microsoft.com/download/pr/2a6bce6b-0e4e-4a0b-a11b-781e2b1a3b24/ca60d047be74b02b25e9a1b6826a2c63/ndp48-devpack-enu.exe"
set "DOTNET_FILE=%TEMP%\ndp48-devpack-enu.exe"

rem Try WebClient first, then BITS as fallback
powershell -NoLogo -NoProfile -Command ^
  "Try { (New-Object Net.WebClient).DownloadFile('%DOTNET_URL%','%DOTNET_FILE%') } Catch { exit 1 }"
if not exist "%DOTNET_FILE%" (
  echo [WARN] WebClient failed; trying BITS...
  powershell -NoLogo -NoProfile -Command ^
    "Try { Start-BitsTransfer -Source '%DOTNET_URL%' -Destination '%DOTNET_FILE%' } Catch { exit 1 }"
)

if not exist "%DOTNET_FILE%" (
  echo [ERROR] Failed to download .NET 4.8 Dev Pack.
  pause
  exit /b 1
)
echo [OK] Downloaded: %DOTNET_FILE%

echo.
echo [STEP 3] Installing .NET Framework 4.8 Developer Pack silently...
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
    if defined version if !version! GEQ 528040 goto :installed_ok
  )
)

echo [ERROR] .NET installation failed or is incomplete.
pause
exit /b 1

:installed_ok
echo [OK] .NET Framework 4.8 installed successfully! (Release=!version!)
pause
exit /b 0
