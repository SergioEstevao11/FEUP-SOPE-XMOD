all: main main.stat

main: main.c
	cc -Wall -o main main.c

main.stat: main.c
	cc -Wall -static -o main.stat main.c
	
clean:
	rm -f main main.stat main.asm main.prec