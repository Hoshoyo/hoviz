@echo off

set LIB_DIR=../../lib
set INCLUDE_DIR=../../include
set SRC_DIR=../../src

set COMPILER_FLAGS=/MD /Zi /nologo /I%INCLUDE_DIR% /I%SRC_DIR%
set LINKER_FLAGS=opengl32.lib gdi32.lib user32.lib shell32.lib %LIB_DIR%/freetype.lib %LIB_DIR%/GLFW/glfw3.lib

if not exist bin ( mkdir bin )
pushd bin
echo "Compiling hoviz"
cl /c %COMPILER_FLAGS% %SRC_DIR%/*.c
popd

echo "Compiling examples"

if not exist 00_basic ( mkdir 00_basic )
pushd 00_basic
cl %COMPILER_FLAGS% ../00_basic.c /link ../bin/*.obj %LINKER_FLAGS%
popd

if not exist 01_anti_alias_lines ( mkdir 01_anti_alias_lines )
pushd 01_anti_alias_lines
cl %COMPILER_FLAGS% ../01_anti_alias_lines.c /link ../bin/*.obj %LINKER_FLAGS%
popd

echo "Copying files do bin directory"
cp ../lib/freetype.dll bin
mv **/*.exe bin
