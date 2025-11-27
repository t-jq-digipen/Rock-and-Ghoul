@echo off
setlocal enabledelayedexpansion

REM ==========================================
REM 1. MENU
REM ==========================================
echo ==========================================
echo   Select Build Configuration
echo ==========================================
echo   1) Debug
echo   2) Release
echo.
set /p choice=Enter choice (1/2): 

if "%choice%"=="1" (
    set CONFIG=Debug
) else if "%choice%"=="2" (
    set CONFIG=Release
) else (
    echo Invalid choice! Defaulting to Release.
    set CONFIG=Release
)

echo.
echo Building configuration: %CONFIG%
echo ==========================================

REM ==========================================
REM 2. Paths
REM ==========================================
set BUILD_DIR=build
set PROJECT_NAME=team_aviddivas

REM ==========================================
REM 3. Create build folder
REM ==========================================
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

cd %BUILD_DIR%

REM ==========================================
REM 4. Run CMake configure step
REM ==========================================
cmake -A x64 ..

REM ==========================================
REM 5. Build
REM ==========================================
cmake --build . --config %CONFIG%

REM ==========================================
REM 6. Run the executable
REM ==========================================
if exist %CONFIG%\%PROJECT_NAME%.exe (
    echo.
    echo Running %PROJECT_NAME%.exe ...
    %CONFIG%\%PROJECT_NAME%.exe
) else (
    echo.
    echo ERROR: Could not find %CONFIG%\%PROJECT_NAME%.exe
)

cd ..
echo.
pause
