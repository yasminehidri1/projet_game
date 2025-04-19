all: prog

prog: main.o fonction.o menus.o
	gcc main.o fonction.o menus.o -o prog -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lm -g

main.o: main.c fonction.h
	gcc -c main.c -g

fonction.o: fonction.c fonction.h 
	gcc -c fonction.c -g
menus.o: menus.c fonction.h 
	gcc -c menus.c -g
clean:
	rm -f *.o prog
