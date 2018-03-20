CC = gcc
CFLAGS = -Wall -std=c99
LIB = ../zad1

static:
	$(CC) $(CFLAGS) main.c -L$(LIB) -ltable -o zad2_staticlib

shared:
	$(CC) $(CFLAGS) main.c -L$(LIB) -Wl,-rpath=$(LIB) -ltable -o zad2_sharedlib

test:


clean:
	rm *.o zad2
	