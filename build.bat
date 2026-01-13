@echo off
set ROOT=%~dp0

echo ================================
echo Building Stunad Engine
echo ================================

cd /d "%ROOT%"

if not exist build (
    mkdir build
)

cd build

cmake ..
cmake --build . --config Release

echo ================================
echo Build finished
echo ================================
pause
