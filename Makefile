CC = gcc
CFLAGS = -Wall

all: xmod

main: xmod.c
	$(CC) -o xmod.o xmod.c

clean:
	rm -f *.o