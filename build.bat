@echo off

if not exist bin ( mkdir bin )
pushd bin
call cl /nologo /MD /Zi /I../include /I../src ../src/*.c ../main.c /Fehoviz.exe /link ../lib/GLFW/glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib ../lib/freetype.lib
cp ../lib/freetype.dll ../bin
popd