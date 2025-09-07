@echo off
cd C:\CollegeStuff\TY\AI\CP

REM Compile the record_audio.cpp file
"C:\msys64\mingw64.exe" "/c/msys64/mingw64/bin/g++.exe" record_audio.cpp -o record_audio -L"C:\Program Files (x86)\portaudio\lib" -lportaudio -I"C:\Program Files (x86)\portaudio\include" -I/mingw64/include

REM Check for compilation errors
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b %errorlevel%
)

REM Run the compiled executable
"C:\msys64\mingw64.exe" "/c/CollegeStuff/TY/AI/CP/record_audio.exe"
pause
