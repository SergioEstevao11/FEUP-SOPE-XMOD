CC = gcc
CFLAGS = -Wall

all: xmod

xmod: xmod.c
	$(CC) -o xmod xmod.c

clean:
	rm -f xmod