CFLAGS=-lraylib -lm -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces

galaxy: main.c 
	${CC} ${CFLAGS} main.c -o galaxy

run: galaxy
	./galaxy

.PHONY: clean

clean:
	${RM} *.o galaxy
