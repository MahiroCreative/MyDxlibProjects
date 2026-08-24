@echo off
setlocal

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1

if not exist build mkdir build

cl.exe /nologo /EHsc /MT /std:c++17 /wd4819 /I DxLib src\*.cpp DxLib\DxLib_x64.lib /Fo:build\ /Fe:build\game.exe /link /LIBPATH:DxLib /SUBSYSTEM:WINDOWS

endlocal
