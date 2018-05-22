CC = gcc
CFLAGS = -Wall -std=gnu99 -lpthread -lrt

make:
	$(CC) barber.c -o barber $(CFLAGS)
	$(CC) clients.c -o clients $(CFLAGS)

clean:
	rm {barber,clients} 2> /dev/null