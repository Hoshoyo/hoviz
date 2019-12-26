all:
	gcc -Iinclude -g src/*.c -o bin/hoviz -lm -lGL -lglfw