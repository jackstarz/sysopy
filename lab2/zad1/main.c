// Napisac czy uzywamy rand() czy /dev/random
// TODO:
// - pomiary
// - sortowanie
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

void generate(const char *, size_t, size_t);
void copy(const char * , const char *, size_t, size_t, int);
int sort_sys(const char *, size_t, size_t);
int sort_lib(const char *, size_t, size_t);

int main(int argc, char *argv[]) {
  generate("data.txt", 20, 12);
  copy("data.txt", "not_sorted.txt", 20, 12, 0);
  sort_lib("data.txt", 20, 12);


  return 0;
}

void generate(const char *path, size_t lines, size_t record_length) {
  int random = open("/dev/urandom", O_RDONLY);
  int rand_val;
  FILE * f = fopen(path, "w+");
  char * record = malloc((record_length + 1) * sizeof(char));

  for (size_t l = 0; l < lines; ++l) {
    for (size_t c = 0; c < record_length; ++c) {
      read(random, &rand_val, sizeof(int));
      rand_val = abs(rand_val) % 26 + 'A';
      record[c] = rand_val;
    }
    record[record_length] = '\n';
    fprintf(f, record);
  }

  close(random);
  fclose(f);
  free(record);

  return;
}

void copy(const char * in_path, const char * out_path, size_t records_amount, size_t record_buf, int use_sys) {
  char * buf = malloc(sizeof(char) * record_buf);  

  if (use_sys) {
    int src = open(in_path, O_RDONLY);
    int dest = open(out_path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    for (size_t i = 0; i < records_amount; ++i) {
      if (read(src, buf, record_buf + 1) != record_buf + 1) {
        return;
      }

      if (write(dest, buf, record_buf + 1) != record_buf + 1) {
        return;
      }
    }

    close(src);
    close(dest);
  } else {
    FILE * src = fopen(in_path, "r");
    FILE * dest = fopen(out_path, "w+");

    for (size_t i = 0; i < records_amount; ++i) {
      if (fread(buf, sizeof(char), record_buf + 1, src) != record_buf + 1) {
        return;
      }

      if (fwrite(buf, sizeof(char), record_buf + 1, dest) != record_buf + 1) {
        return;
      }
    } 
    fclose(src);
    fclose(dest);
  }
  
  free(buf);
}

int sort_sys(const char * path, size_t lines, size_t record_length) {
  int f = open(path, O_RDWR);
  char * rec1 = malloc(sizeof(char) * (record_length + 1));
  char * rec2 = malloc(sizeof(char) * (record_length + 1));
  off_t offset = (off_t) (sizeof(char) * (record_length + 1));

  for (size_t l = 0; l < lines; ++l) {
    lseek(f, l * offset, SEEK_SET);

    if (read(f, rec1, sizeof(char) * (record_length + 1)) != (record_length + 1)) {
      fprintf(stderr, "Failed to read data while sorting.\n");
      return EXIT_FAILURE;
    }

    for (size_t k = 0; k < l; ++k) {
      lseek(f, k * offset, SEEK_SET);

      if (read(f, rec2, sizeof(char) * (record_length + 1)) != (record_length + 1)) {
        fprintf(stderr, "Failed to read data while sorting.\n");
        return EXIT_FAILURE;
      }

      if (rec2[0] > rec1[0]) {
        lseek(f, k * offset, SEEK_SET);

        if (write(f, rec1, sizeof(char) * (record_length + 1)) != (record_length + 1)) {
          fprintf(stderr, "Failed to write data while sorting.\n");
          return EXIT_FAILURE;
        }

        lseek(f, l * offset, SEEK_SET);

        if (write(f, rec2, sizeof(char) * (record_length + 1)) != (record_length + 1)) {
          fprintf(stderr, "Failed to write data while sorting.\n");
          return EXIT_FAILURE;
        }

        char * tmp = rec2;
        rec2 = rec1;
        rec1 = tmp;
      }
    }
  }

  close(f);
  free(rec1);
  free(rec2);

  return EXIT_SUCCESS;
}

int sort_lib(const char * path, size_t lines, size_t record_length) {
  // dlaczego "rw" nie dziala?
  FILE * f = fopen(path, "r+");
  char * rec1 = malloc(sizeof(char) * (record_length + 1));
  char * rec2 = malloc(sizeof(char) * (record_length + 1));
  off_t offset = (off_t) ((sizeof(char) * record_length + 1));

  for (size_t l = 0; l < lines; ++l) {
    fseek(f, l * offset, 0);

    if (fread(rec1, sizeof(char), (record_length + 1), f) != record_length + 1) {
      fprintf(stderr, "Failed to read data while sorting.\n");
      return EXIT_FAILURE;
    }

    for (size_t k = 0; k < l; ++k) {
      fseek(f, k * offset, 0);

      if (fread(rec2, sizeof(char), (record_length + 1), f) != record_length + 1) {
        fprintf(stderr, "Failed to read data while sorting.\n");
        return EXIT_FAILURE;
      }

      if (rec2[0] > rec1[0]) {
        fseek(f, k * offset, 0);

        if (fwrite(rec1, sizeof(char), (record_length + 1), f) != record_length + 1) {
          fprintf(stderr, "Failed to write data while sorting.\n");
          return EXIT_FAILURE;
        }

        fseek(f, l * offset, 0);

        if (fwrite(rec2, sizeof(char), (record_length + 1), f) != record_length + 1) {
          fprintf(stderr, "Failed to write data while sorting.\n");
          return EXIT_FAILURE;
        }

        char * tmp = rec2;
        rec2 = rec1;
        rec1 = tmp;
      }
    }
  }

  fclose(f);
  free(rec1);
  free(rec2);

  return EXIT_SUCCESS;
}