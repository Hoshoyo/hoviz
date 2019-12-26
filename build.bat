@echo off

pushd bin
call cl /nologo /MD /Zi /I../include ../src/*.c /Fehoviz.exe /link ../lib/GLFW/glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib
popd