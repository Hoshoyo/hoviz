all:
	gcc -Iinclude -g src/*.c -o bin/hoviz -lm -lGL lib/libfreetype.a -lglfw
static-lib: extract-freetype
	cd bin; gcc -DIMPLEMENT_GL_GM=1 -I../include -c ../src/batcher.c ../src/camera.c ../src/font_load.c ../src/font_render.c ../src/hoviz.c ../src/input.c ../src/os.c ../src/quaternion.c ../src/shader.c
	cd bin; ar rcs libhoviz.a *.o temp/*.o

extract-freetype:
	mkdir -p bin/temp
	cd bin/temp; ar -x ../../lib/libfreetype.a

clean:
	rm -rf bin/temp