@echo off
set BUILD_DIR=build
set CONFIG=Debug
set PROJECT_NAME=team_aviddivas

REM 1. Create build folder if it doesn't exist
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

REM 2. Run CMake configure step
cd %BUILD_DIR%
cmake ..

REM 3. Build with the chosen config
cmake --build . --config %CONFIG%

REM 4. Run the executable (if it exists)
if exist %CONFIG%\%PROJECT_NAME%.exe (
    %CONFIG%\%PROJECT_NAME%.exe
) else (
    echo ERROR: Could not find %CONFIG%\%PROJECT_NAME%.exe
)

cd ..
pause
