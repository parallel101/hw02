cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --target ALL_BUILD --config Release
.\build\bin\main.exe
pause