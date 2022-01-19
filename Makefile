all: malloc.so

malloc.so: malloc.o
	gcc -shared -ldl malloc.o -o malloc.so

malloc.o: malloc.c
	gcc -c -fPIC -ldl malloc.c -o malloc.o

clean:
	rm malloc.so malloc.o

run:
	LD_PRELOAD=malloc.so seq 1 5
