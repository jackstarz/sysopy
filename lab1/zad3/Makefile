CC = gcc
CFLAGS = -Wall
SRC = ./main.c
LIB = ../zad1

zad3_static:
	$(CC) $(CFLAGS) $(SRC) -L$(LIB) -ltable -o zad3_libstatic

zad3_shared:
	$(CC) $(CFLAGS) $(SRC) -L$(LIB) -Wl,-rpath=$(LIB) -ltable -o zad3_libshared

zad3_dynamic:
	$(CC) $(CFLAGS) -DDYNAMIC $(SRC) -ldl -o zad3_libdynamic

clean:
	rm *.o zad3
	