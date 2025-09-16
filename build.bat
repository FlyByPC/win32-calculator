@echo off
setlocal

set "VSROOT=C:\Program Files\Microsoft Visual Studio\2022\Community"
if not exist "%VSROOT%\Common7\Tools\VsDevCmd.bat" (
    echo Could not find VsDevCmd.bat at "%VSROOT%\Common7\Tools".>&2
    echo Update build.bat with the correct path to your Visual Studio installation.>&2
    exit /b 1
)

call "%VSROOT%\Common7\Tools\VsDevCmd.bat" -arch=x64
if errorlevel 1 (
    echo Failed to initialize the Visual Studio Developer environment.>&2
    exit /b 1
)

if not exist build mkdir build
cl /std:c17 /W4 /EHsc /DUNICODE /D_UNICODE src\WinCalc.c user32.lib gdi32.lib /Fe:build\WinCalc.exe
exit /b %errorlevel%
