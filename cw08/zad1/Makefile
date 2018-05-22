CC = gcc
CFLAGS = -Wall -std=c99 -lm -lpthread

make:
	$(CC) main.c -o filter $(CFLAGS)
	$(CC) gen_filter.c -o gen_filter

test:
	./gen_filter 25
	./gen_filter 50
	./filter 1 lena.pgm fltr_25 out.pgm
	./filter 2 lena.pgm fltr_25 out.pgm
	./filter 4 lena.pgm fltr_25 out.pgm
	./filter 8 lena.pgm fltr_25 out.pgm
	./filter 1 lena.pgm fltr_50 out.pgm
	./filter 2 lena.pgm fltr_50 out.pgm
	./filter 4 lena.pgm fltr_50 out.pgm
	./filter 8 lena.pgm fltr_50 out.pgm

clean:
	rm filter gen_filter fltr* 2>/dev/null