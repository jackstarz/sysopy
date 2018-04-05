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
	./zad2 500000 32 s search 420
	./zad2 500000 32 s swap 200000
	./zad2 500000 32 s alt_swap 20000
	./zad2 500000 32 d search 420
	./zad2 500000 32 d swap 20000
	./zad2 500000 32 d alt_swap 20000

	./zad2 500 4096 s search 420
	./zad2 500 4096 s swap 200
	./zad2 500 4096 s alt_swap 200
	./zad2 500 4096 d search 420
	./zad2 500 4096 d swap 200
	./zad2 500 4096 d alt_swap 200

	