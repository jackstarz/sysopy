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

int main(int argc, char *argv[]) {
  generate("data.txt", 100, 512);
  copy("data.txt", "out.txt", 10, 512, 0);

  return 0;
}

void generate(const char *path, size_t lines, size_t record_length) {
  int random = open("/dev/urandom", O_RDONLY);
  int rand_val;
  FILE * out = fopen(path, "w+");
  char * record = malloc((record_length + 1) * sizeof(char));

  for (size_t l = 0; l < lines; ++l) {
    for (size_t c = 0; c < record_length; ++c) {
      read(random, &rand_val, sizeof(int));
      rand_val = abs(rand_val) % 26 + 'A';
      record[c] = rand_val;
    }
    record[record_length] = '\n';
    fprintf(out, record);
  }

  close(random);
  fclose(out);
  free(record);

  return;
}

void copy(const char * in_path, const char * out_path, size_t records_amount, size_t record_size, int use_sys) {
  char * buf = malloc(sizeof(char) * record_size);  

  if (use_sys) {
    int src = open(in_path, O_RDONLY);
    int dest = open(out_path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    for (size_t i = 0; i < records_amount; ++i) {
      if (read(src, buf, record_size + 1) != record_size + 1) {
        return;
      }

      if (write(dest, buf, record_size + 1) != record_size + 1) {
        return;
      }
    }

    close(src);
    close(dest);
  } else {
    FILE * src = fopen(in_path, "r");
    FILE * dest = fopen(out_path, "w+");

    for (size_t i = 0; i < records_amount; ++i) {
      if (fread(buf, sizeof(char), record_size + 1, src) != record_size + 1) {
        return;
      }

      if (fwrite(buf, sizeof(char), record_size + 1, dest) != record_size + 1) {
        return;
      }
    } 
    fclose(src);
    fclose(dest);
  }
  
  free(buf);
}