@echo off

set lflags= SDL2.lib SDL2main.lib Shell32.lib User32.lib Shcore.lib

set sdl2_inlcude= /I..\SDL2\include\
set stb_include= /I..\STB\

set src= /I..\src\

set exec= app.exe

if not exist bin mkdir bin
pushd bin

if exist %exec% del %exec%

cl /nologo /Zi ..\src\app.c %sdl2_inlcude% %src% %stb_include% /link /incremental:no /subsystem:console /libpath:..\SDL2\lib\x64 %lflags%

popd bin
