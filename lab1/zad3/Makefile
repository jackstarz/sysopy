CC = gcc
CFLAGS = -Wall
SRC = ./main.c
LIB = ../zad1

all: static shared dynamic

static:
	$(CC) $(CFLAGS) $(SRC) -L$(LIB) -ltable -o zad3_libstatic

shared:
	$(CC) $(CFLAGS) $(SRC) -L$(LIB) -Wl,-rpath=$(LIB) -ltable -o zad3_libshared

dynamic:
	$(CC) $(CFLAGS) -DDYNAMIC $(SRC) -ldl -o zad3_libdynamic

clean:
	rm *.o zad3_*
	