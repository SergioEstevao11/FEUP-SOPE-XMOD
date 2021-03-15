CC = gcc
CFLAGS = -Wall

all: xmod

xmod: xmod.c xmod_aux.c
	$(CC) -o xmod xmod.c xmod_aux.c

clean:
	rm -f xmod