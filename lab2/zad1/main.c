// Napisac czy uzywamy rand() czy /dev/random
// TODO:
// - pomiary
// - sortowanie
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SYS 1
#define LIB 0

typedef struct {
  const char * path;
  size_t rec_amount;
  size_t rec_length;
} Records_File;

typedef struct {
  struct timeval user;
  struct timeval system;
} Timing;

int generate(Records_File *);
int sort(Records_File *, int);
int copy(Records_File *, const char *, int);
void print_usage();
Timing get_timing();
void end_timing(Timing * t);
void save_timing(FILE *, Timing * t);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage();
    return EXIT_FAILURE;
  }

  Records_File file;
  FILE * results = fopen("wyniki.txt", "a");
  Timing t;

  char * op = argv[1];

  if (strcmp(op, "generate") == 0) {
    if (argc < 5) {
      fprintf(stderr, "Please specify correct arguments.\n\n");
      return EXIT_FAILURE;
    }

    file.path = argv[2];
    file.rec_amount = strtol(argv[3], 0 ,10);
    file.rec_length = strtol(argv[4], 0, 10);
    fprintf(results, "generating file with %ld records of size %ld.\n", 
            file.rec_amount, file.rec_length);
    t = get_timing();  
    generate(&file);
    end_timing(&t);
    save_timing(results, &t);
  } else if (strcmp(op, "sort") == 0) {
    file.path = argv[2];
    file.rec_amount = strtol(argv[3], 0 ,10);
    file.rec_length = strtol(argv[4], 0, 10);
    int mode = strcmp(argv[5], "sys") == 0 ? SYS : LIB;
    fprintf(results, "sorting file with %ld record of size %ld, using %s functions.\n",
            file.rec_amount, file.rec_length, mode == SYS ? "system" : "library");
    t = get_timing();
    sort(&file, mode);
    end_timing(&t);
    save_timing(results, &t);
  } else if (strcmp(op, "copy") == 0) {
    file.path = argv[2];
    const char * out = argv[3];
    file.rec_amount = strtol(argv[4], 0, 10);
    file.rec_length = strtol(argv[5], 0, 10);    
    int mode = strcmp(argv[6], "sys") == 0  ? SYS : LIB;
    fprintf(results, "copying %ld lines from file with %ld bytes long buffer, using %s functions.\n",
            file.rec_amount, file.rec_length, mode == SYS ? "system" : "library");
    t = get_timing();
    copy(&file, out, mode);
    end_timing(&t);
    save_timing(results, &t);
  } else {
    fprintf(stderr, "Wrong operation specified.\n\n");
    print_usage();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int generate(Records_File * file) {
  int random = open("/dev/urandom", O_RDONLY);
  if (!random) {
    fprintf(stderr, "Failed to open /dev/urandom.\n");
    return EXIT_FAILURE;
  }

  int rand_val;

  FILE * f = fopen(file->path, "w+");
  if (!f) {
    fprintf(stderr, "Failed to open output file %s", file->path);
    return EXIT_FAILURE;
  }

  char * record = malloc((file->rec_length + 1) * sizeof(char));
  if (!record) {
    fprintf(stderr, "Failed to alocate memory for record.\n");
    return EXIT_FAILURE;
  }

  for (size_t l = 0; l < file->rec_amount; ++l) {
    for (size_t c = 0; c < file->rec_length; ++c) {
      if (read(random, &rand_val, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Failed to read data from /dev/urandom.\n");
        return EXIT_FAILURE;
      }
      rand_val = abs(rand_val) % 26 + 'A';
      record[c] = rand_val;
    }
    record[file->rec_length] = '\n';
    fprintf(f, record);
  }

  close(random);
  fclose(f);
  free(record);

  return EXIT_SUCCESS;
}

int copy(Records_File * file, const char * out, int mode) {
  char * buf = malloc(sizeof(char) * (file->rec_length + 1));
  if (!buf) {
    fprintf(stderr, "Failed to alocate memory for buffer.\n");
    return EXIT_FAILURE;
  }

  if (mode == SYS) {
    int src = open(file->path, O_RDONLY);
    if (!src) {
      fprintf(stderr, "Failed to open source file %s.\n", file->path);
      return EXIT_FAILURE;
    }

    int dest = open(out, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if (!dest) {
      fprintf(stderr, "Failed to open destination file %s.\n", out);
      return EXIT_FAILURE;
    }

    for (size_t i = 0; i < file->rec_amount; ++i) {
      if (read(src, buf, (file->rec_length + 1)) != file->rec_length + 1) {
        fprintf(stderr, "Failed to read data while copying.\n");
        return EXIT_FAILURE;
      }

      if (write(dest, buf, (file->rec_length + 1)) != file->rec_length + 1) {
        fprintf(stderr, "Failed to write data while copying.\n");
        return EXIT_FAILURE;
      }
    }

    close(src);
    close(dest);
  } else {
    FILE * src = fopen(file->path, "r");
    if (!src) {
      fprintf(stderr, "Failed to open source file %s.\n", file->path);
      return EXIT_FAILURE;
    }

    FILE * dest = fopen(out, "w+");
    if (!dest) {
      fprintf(stderr, "Failed to open destination file %s.\n", out);
      return EXIT_FAILURE;
    }

    for (size_t i = 0; i < file->rec_amount; ++i) {
      if (fread(buf, sizeof(char), (file->rec_length + 1), src) != file->rec_length + 1) {
        fprintf(stderr, "Failed to read data while copying.\n");
        return EXIT_FAILURE;
      }

      if (fwrite(buf, sizeof(char), (file->rec_length + 1), dest) != file->rec_length + 1) {
        fprintf(stderr, "Failed to write data while copying.\n");
        return EXIT_FAILURE;
      }
    } 
    fclose(src);
    fclose(dest);
  }
  
  free(buf);

  return EXIT_SUCCESS;
}

int sort(Records_File * file, int mode) {
  char * rec1 = malloc(sizeof(char) * (file->rec_length + 1));
  char * rec2 = malloc(sizeof(char) * (file->rec_length + 1));

  if (!rec1 || !rec2) {
    fprintf(stderr, "Failed to alocate memory for records.\n");
    return EXIT_FAILURE;
  }

  off_t offset = (off_t) (sizeof(char) * (file->rec_length + 1));

  if (mode == SYS) {
    int f = open(file->path, O_RDWR);

    for (size_t l = 0; l < file->rec_amount; ++l) {
      lseek(f, l * offset, SEEK_SET);

      if (read(f, rec1, sizeof(char) * (file->rec_length + 1)) != file->rec_length + 1) {
        fprintf(stderr, "Failed to read data while sorting.\n");
        return EXIT_FAILURE;
      }

      for (size_t k = 0; k < l; ++k) {
        lseek(f, k * offset, SEEK_SET);

        if (read(f, rec2, sizeof(char) * (file->rec_length + 1)) != file->rec_length + 1) {
          fprintf(stderr, "Failed to read data while sorting.\n");
          return EXIT_FAILURE;
        }

        if (rec2[0] > rec1[0]) {
          lseek(f, k * offset, SEEK_SET);

          if (write(f, rec1, sizeof(char) * (file->rec_length + 1)) != file->rec_length + 1) {
            fprintf(stderr, "Failed to write data while sorting.\n");
            return EXIT_FAILURE;
          }

          lseek(f, l * offset, SEEK_SET);

          if (write(f, rec2, sizeof(char) * (file->rec_length + 1)) != file->rec_length + 1) {
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
  } else {

    FILE * f = fopen(file->path, "r+");

    for (size_t l = 0; l < file->rec_amount; ++l) {
      fseek(f, l * offset, 0);

      if (fread(rec1, sizeof(char), (file->rec_length + 1), f) != file->rec_length + 1) {
        fprintf(stderr, "Failed to read data while sorting.\n");
        return EXIT_FAILURE;
      }

      for (size_t k = 0; k < l; ++k) {
        fseek(f, k * offset, 0);

        if (fread(rec2, sizeof(char), (file->rec_length + 1), f) != file->rec_length + 1) {
          fprintf(stderr, "Failed to read data while sorting.\n");
          return EXIT_FAILURE;
        }

        if (rec2[0] > rec1[0]) {
          fseek(f, k * offset, 0);

          if (fwrite(rec1, sizeof(char), (file->rec_length + 1), f) != file->rec_length + 1) {
            fprintf(stderr, "Failed to write data while sorting.\n");
            return EXIT_FAILURE;
          }

          fseek(f, l * offset, 0);

          if (fwrite(rec2, sizeof(char), (file->rec_length + 1), f) != file->rec_length + 1) {
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
  }

  free(rec1);
  free(rec2);

  return EXIT_SUCCESS;
}

void print_usage() {
  printf("Usage: ./zad1 operation args\n"
         "  generate <out> <len> <size>        - creates <out> file with <len> records, <size> bytes long each.\n"
         "  sort <in> <len> <size> <m>         - sorts <in> file assuming it has <len> records, <size> bytes long each, using <m> mode (sys or lib).\n"
         "  copy <in> <out> <len> <size> <m>   - copies <len> record, <size> bytes each from <in> file to <out> file using <m> mode (sys or lib).\n");
}

Timing get_timing() {
  Timing t;
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);
  t.system = ru.ru_stime;
  t.user = ru.ru_utime;

  return t;
}

void end_timing(Timing * t) {
  struct rusage current_usage;
  getrusage(RUSAGE_SELF, &current_usage);

  t->user.tv_sec = current_usage.ru_utime.tv_sec - t->user.tv_sec;
  t->user.tv_usec = current_usage.ru_utime.tv_usec - t->user.tv_usec;
  t->system.tv_sec = current_usage.ru_stime.tv_sec - t->system.tv_sec;
  t->system.tv_usec = current_usage.ru_stime.tv_usec - t->system.tv_usec;
}

void save_timing(FILE * f, Timing * t) {
  fprintf(f, "  user: %.6f, system: %.6f\n\n",
         t->user.tv_sec + t->user.tv_usec / 10e6,
         t->system.tv_sec + t->system.tv_usec / 10e6);
}