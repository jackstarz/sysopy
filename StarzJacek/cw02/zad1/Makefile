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

	./zad1 generate data 10000 4
	./zad1 copy data copied_data_sys 10000 4 sys
	./zad1 copy data copied_data_lib 10000 4 lib
	./zad1 sort copied_data_sys 10000 4 sys
	./zad1 sort copied_data_lib 10000 4 lib

	./zad1 generate data 5000 512
	./zad1 copy data copied_data_sys 5000 512 sys
	./zad1 copy data copied_data_lib 5000 512 lib
	./zad1 sort copied_data_sys 5000 512 sys
	./zad1 sort copied_data_lib 5000 512 lib

	./zad1 generate data 10000 512
	./zad1 copy data copied_data_sys 10000 512 sys
	./zad1 copy data copied_data_lib 10000 512 lib
	./zad1 sort copied_data_sys 10000 512 sys
	./zad1 sort copied_data_lib 10000 512 lib

	./zad1 generate data 5000 4096
	./zad1 copy data copied_data_sys 5000 4096 sys
	./zad1 copy data copied_data_lib 5000 4096 lib
	./zad1 sort copied_data_sys 5000 4096 sys
	./zad1 sort copied_data_lib 5000 4096 lib

	./zad1 generate data 10000 4096
	./zad1 copy data copied_data_sys 10000 4096 sys
	./zad1 copy data copied_data_lib 10000 4096 lib
	./zad1 sort copied_data_sys 10000 4096 sys
	./zad1 sort copied_data_lib 10000 4096 lib
	
	./zad1 generate data 5000 8192
	./zad1 copy data copied_data_sys 5000 8192 sys
	./zad1 copy data copied_data_lib 5000 8192 lib
	./zad1 sort copied_data_sys 5000 8192 sys
	./zad1 sort copied_data_lib 5000 8192 lib

	./zad1 generate data 10000 8192
	./zad1 copy data copied_data_sys 10000 8192 sys
	./zad1 copy data copied_data_lib 10000 8192 lib
	./zad1 sort copied_data_sys 10000 8192 sys
	./zad1 sort copied_data_lib 10000 8192 lib