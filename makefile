all:
	gcc -g -Wall -Wextra demon.c -o demon.exe

clean:
	rm demon.exe