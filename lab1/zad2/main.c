#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../zad1/table.h"

#ifndef DYNAMIC
#define STATIC
#endif

typedef struct timeval timeval;
typedef struct rusage rusage;

typedef struct {
  clock_t real;
  timeval user;
  timeval system;
} Timing;

void print_usage();
char * generate_block(size_t size);
void exec_and_save(FILE * f, Table * a, char * op, int arg);
size_t search_element(Table * a, size_t value);
void remove_block(Table * a, size_t number);
void add_blocks(Table * a, size_t number);
void remove_blocks(Table * a, size_t number);

Timing get_timing();
void end_timing(Timing * t);
void save_timing(FILE * f, Timing * t);

int main(int argc, char** argv) {
 
  printf("tu spokos");

  if (argc < 4) {
    print_usage();
    return -1;
  }

  size_t len = strtol(argv[1], 0, 10);
  if (len == 0) {
    fprintf(stderr, "Please give length as positive integer.\n");
    return -1;
  }

  size_t size = strtol(argv[2], 0, 10);
  if (size == 0) {
    fprintf(stderr, "Please give block size as positive integer.\n");
    return -1;
  }

  int is_static;
  if (strcmp(argv[3], "s") == 0) {
    is_static = 1;
  } else if (strcmp(argv[3], "d") == 0) {
    is_static = 0;
  } else {
    fprintf(stderr, "Please declare alocation type as s (static) or d (dynamic).\n");
    return -1;
  }

  char * operation = NULL;
  size_t arg = 0; 

  FILE * report;
  report = fopen("raport2.txt", "a");

  Timing t = get_timing();
  Table * a = create_table(len, size, is_static);
  for (size_t i = 0; i < a->length; i++) {
    char * block = generate_block(a->block_size);
    add_block(a, i, block);
  }
  end_timing(&t);
  fprintf(report, "Creating table with parameters:\n"
          "length: %ld, block size: %ld, alocation method: %s\n",
          a->length, a->block_size, is_static ? "static" : "dynamic");
  save_timing(report, &t);

  if (argc >= 5) {
    operation = argv[4];
  }
  if (argc >= 6) {
    arg = strtol(argv[5], 0, 10);
    exec_and_save(report, a, operation, arg);
  }

  fclose(report);
  fprintf(report, "\n-------------------------------------------------\n");

  return 0;
}

void print_usage() {
  printf("Usage: zad2 length block_size alocation operation\n"
         "  length - length of table.\n"
         "  block_size - size of each block.\n"
         "  alocation - alocation type [s|d] s - static, d - dynamic.\n"
         "  operation - operation to execute. \n"
         "    * search value - search block of sum similar to value.\n"      
         "    * change value - remove value blocks, then add new ones.\n"
         "    * alt_change number - remove then add one block number times.\n");
}

char * generate_block(size_t size) {
  char * block = calloc(size, sizeof(char));
  for (size_t i = 0; i < size; i++) {
    block[i] = rand() % ('z' - 'A') + 'A' + 1;
  }
  return block;
}

void exec_and_save(FILE * f, Table * a, char * op, int arg) {
  Timing t;
  if (strcmp(op, "search") == 0) {
    fprintf(f, "\nSearch for element with sum: %d\n", arg);
    t = get_timing();
    size_t i = search_block(a, arg);
    end_timing(&t);
    save_timing(f, &t);
    fprintf(f, "Element found on index: %ld, with sum %ld.\n", i, block_sum(a->blocks[i]));
  } else if (strcmp(op, "change") == 0) {
    fprintf(f, "\nRemove element from index: %d\n", arg);
    t = get_timing();
    remove_blocks(a, arg);
    add_blocks(a, arg);
    end_timing(&t);
    save_timing(f, &t);
  } else if (strcmp(op, "alt_change") == 0) {
    fprintf(f, "\nAdd element to index: %d\n", arg);
    char * block = generate_block(a->block_size);
    t = get_timing();
    for (size_t b = 0; b < arg; b++) {
      delete_block(a, 0);
      add_block(a, 0, block);
    }
    end_timing(&t);
    save_timing(f, &t);
  } else {
    fprintf(stderr, "Wrong operation.\n");
  }
}

void remove_blocks(Table * a, size_t number) {
  if (number > a->length) {
    fprintf(stderr, "Error. Blocks number cannot be greater than table length.\n");
    return;
  }

  for (size_t b = 0; b < number; b++) {
    delete_block(a, number);
  }
}

void add_blocks(Table * a, size_t number) {
  if (number > a->length) {
    fprintf(stderr, "Error. Blocks number cannot be greater than table length.\n");
    return;
  }

  char *  block = generate_block(a->block_size);
  for (size_t b = 0; b < number; b++) {
   add_block(a, number, block);
  }
}

void time_check(Timing * t) {
  clock_t curr;
  struct rusage curr_ru;

  curr = clock();
  getrusage(RUSAGE_SELF, &curr_ru);
  printf("  real: %.6f, user: %.6f, system: %.6f\n",
    (double)(curr) / CLOCKS_PER_SEC,
    curr_ru.ru_utime.tv_sec + curr_ru.ru_utime.tv_usec / 10e6,
    curr_ru.ru_stime.tv_sec + curr_ru.ru_stime.tv_usec / 10e6);
}

Timing get_timing() {
  Timing t;
  rusage ru;

  t.real = clock();
  getrusage(RUSAGE_SELF, &ru);
  t.system = ru.ru_stime;
  t.user = ru.ru_utime;

  return t;
}

void end_timing(Timing * t) {
  clock_t current_real = clock();
  rusage current_usage;
  getrusage(RUSAGE_SELF, &current_usage);

  t->real = current_real - t->real;
  t->user.tv_sec = current_usage.ru_utime.tv_sec - t->user.tv_sec;
  t->user.tv_usec = current_usage.ru_utime.tv_usec - t->user.tv_usec;
  t->system.tv_sec = current_usage.ru_stime.tv_sec - t->system.tv_sec;
  t->system.tv_usec = current_usage.ru_stime.tv_usec - t->system.tv_usec;
}

void save_timing(FILE * f, Timing * t) {
  fprintf(f, "  real: %.6f, user: %.6f, system: %.6f\n",
         (double) t->real / CLOCKS_PER_SEC,
         t->user.tv_sec + t->user.tv_usec / 10e6,
         t->system.tv_sec + t->system.tv_usec / 10e6);
}
