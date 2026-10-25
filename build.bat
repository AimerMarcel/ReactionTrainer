@echo off
REM Builds ReactionTrainer.exe. Uses MinGW-w64 g++ if found, otherwise MSVC cl (run from a VS Developer Command Prompt).
cd /d "%~dp0"
where g++ >nul 2>&1
if %errorlevel%==0 (
  echo Building with MinGW g++ ...
  g++ -std=c++17 -O2 -mwindows -static -static-libgcc -static-libstdc++ -o ReactionTrainer.exe src\main_win32.cpp -luser32 -lgdi32 -lole32 -loleaut32 -luuid
  goto done
)
where cl >nul 2>&1
if %errorlevel%==0 (
  echo Building with MSVC cl ...
  cl /nologo /O2 /EHsc /std:c++17 /MT src\main_win32.cpp user32.lib gdi32.lib ole32.lib oleaut32.lib uuid.lib /link /SUBSYSTEM:WINDOWS /OUT:ReactionTrainer.exe
  goto done
)
echo No compiler found. Install MinGW-w64 (https://winlibs.com) or Visual Studio Build Tools, then run this again.
pause
exit /b 1
:done
if exist ReactionTrainer.exe (echo. & echo Done: ReactionTrainer.exe) else (echo Build failed.)
pause
