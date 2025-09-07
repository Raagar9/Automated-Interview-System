@echo off
cd C:\CollegeStuff\TY\AI\CP

REM Navigate to the whisper.cpp directory
cd C:\msys64\home\anees\whisper.cpp

REM Run the compiled executable
"C:\msys64\mingw64.exe" "/c/msys64/home/anees/whisper.cpp/main.exe" -otxt /c/CollegeStuff/TY/AI/CP/output.wav
if %errorlevel% neq 0 (
    echo Execution failed.
    pause
    exit /b %errorlevel%
)

pause
