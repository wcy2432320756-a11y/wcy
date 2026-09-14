@echo off
REM Build the geometry drawing app with MSVC + EasyX.
REM Sources are UTF-8, so cl needs /utf-8 for the Chinese string literals.

setlocal
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\Installer;%PATH%"
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% (
    echo [ERROR] vcvars64.bat not found. Is VS Build Tools installed?
    exit /b 1
)

call %VCVARS% >nul
if errorlevel 1 (
    echo [ERROR] Failed to initialize the MSVC environment.
    exit /b 1
)

cd /d "%~dp0"
if not exist build mkdir build

REM /utf-8   : treat sources as UTF-8
REM /EHsc    : standard C++ exception model
REM /SUBSYSTEM:WINDOWS with /ENTRY:mainCRTStartup keeps main() but hides the console
cl /nologo /EHsc /utf-8 /DUNICODE /D_UNICODE /D_WIN32_WINNT=0x0601 /DWINVER=0x0601 /W4 /wd4201 /O2 /std:c++17 /Fo:build\ ^
   src\main.cpp src\shape.cpp src\ui.cpp ^
   /Fe:build\geodraw.exe ^
   /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup ^
   user32.lib gdi32.lib gdiplus.lib comdlg32.lib ole32.lib oleaut32.lib shell32.lib imm32.lib msimg32.lib

if errorlevel 1 (
    echo.
    echo [FAILED] Compilation did not succeed.
    exit /b 1
)

echo.
echo [OK] Built build\geodraw.exe
endlocal
