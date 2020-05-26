all:
	gcc -g -Wall -Wextra demon.c -lm -o demon.exe

clean:
	rm demon.exe