CC = gcc
CFLAGS = -Wall

all: main

main: main.c
	$(CC) -o main.o main.c

clean:
	rm -f *.o