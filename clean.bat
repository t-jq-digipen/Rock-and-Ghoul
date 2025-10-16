@echo off
set BUILD_DIR=build
set INSTALL_DIR=install

echo Cleaning project...
if exist %BUILD_DIR% (
    rmdir /s /q %BUILD_DIR%
)
if exist %INSTALL_DIR% (
    rmdir /s /q %INSTALL_DIR%
)

echo Done.
pause
