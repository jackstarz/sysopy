CC = gcc
CFLAGS = -Wall -std=c99

all: static shared

zad1: table.o
	$(CC) $(CFLAGS) -o zad1 table.o -I.

static:
	$(CC) $(CFLAGS) -c table.c -o table.o
	ar rcs libtable.a table.o

shared:
	$(CC) $(CFLAGS) -fPIC -shared table.c -o table.so

clean:
	rm *.o *.so *.a
	