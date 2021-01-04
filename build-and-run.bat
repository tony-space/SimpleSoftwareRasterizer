@echo off

mkdir build-x64
cd build-x64
cmake ..
cmake --build . --config Release
x64\Release\host-gdi.exe
cd ..