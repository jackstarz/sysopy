CC = gcc
CFLAGS = -Wall -std=gnu99

make:
	$(CC) $(CFLAGS) barber.c -o barber
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm {barber,client} 2> /dev/null