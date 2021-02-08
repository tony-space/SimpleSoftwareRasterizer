@echo off

mkdir build-x64
cd build-x64
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
x64\Release\host-gdi.exe
cd ..