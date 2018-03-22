CC = gcc
CFLAGS = -Wall -std=c99

make:
	$(CC) $(CFLAGS) main.c -o zad1

clean:
	rm *.txt zad1

test:
	./zad1 generate data 5000 4
	./zad1 copy data copied_data_sys 5000 4 sys
	./zad1 copy data copied_data_lib 5000 4 lib
	./zad1 sort copied_data_sys 5000 4 sys
	./zad1 sort copied_data_lib 5000 4 lib	