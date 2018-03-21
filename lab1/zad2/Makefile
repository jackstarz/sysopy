CC = gcc
CFLAGS = -Wall -std=c99
LIB = ../zad1

static:
	$(CC) $(CFLAGS) main.c -L$(LIB) -ltable -o zad2

shared:
	$(CC) $(CFLAGS) main.c -L$(LIB) -Wl,-rpath=$(LIB) -ltable -o zad2

clean:
	rm zad2_* raport2.txt

test:
	./zad2 500000 32 s search 2137
	./zad2 500000 32 s swap 20000
	./zad2 500000 32 s alt_swap 2137
	./zad2 500000 32 d search 2137
	./zad2 500000 32 d swap 20000
	./zad2 500000 32 d alt_swap 2137
	