all: main main.stat main.asm main.prec
main: main.c
cc -Wall -o main main.c
main.stat: main.c
cc -Wall -static -o main.stat main.c
main.asm: main.c
cc -Wall -S -o main.asm main.c
main.prec: main.c
cc -Wall -E -o main.prec main.c
clean:
rm -f main main.stat main.asm main.prec