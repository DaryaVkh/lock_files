all: lock

CFLAGS=-c -W -Wall -O3
CC=gcc

lock: main.o
	${CC} -o lock main.o -lm

main.o: main.c
	${CC} ${CFLAGS} main.c

clean:
	rm -f *.o lock