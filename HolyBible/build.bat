@echo off
REM Build the Holy Bible on Windows (MinGW g++ or Visual Studio's cl).
where g++ >nul 2>nul
if %errorlevel%==0 (
    g++ -std=c++17 -O2 -o holybible.exe src\main.cpp
) else (
    cl /nologo /std:c++17 /O2 /EHsc /utf-8 src\main.cpp /Fe:holybible.exe
)
if %errorlevel%==0 (
    echo.
    echo Built holybible.exe  -  run it with:  holybible
)
