CC = gcc
CFLAGS = -Wall -std=c99 -lrt

make:
	$(CC) $(CFLAGS) server.c -o server
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm server client