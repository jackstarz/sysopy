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

typedef struct timeval timeval;
typedef struct rusage rusage;

typedef struct {
  clock_t real;
  timeval user;
  timeval system;
} Timing;

void print_usage();
void exec_and_save(FILE *, Table *, char *, int);
void block_del(Table *, size_t);
void block_add(Table *, size_t);
char * block_gen(size_t);
Table * table_create(size_t, size_t, int);

size_t search(Table *, size_t);
void swap(Table *, size_t);
void alt_swap(Table *, size_t);

Timing get_timing();
void end_timing(Timing *);
void save_timing(FILE *, Timing *);

int main(int argc, char** argv) {
 
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
  srand(time(NULL));
  Table * a = table_create(len, size, is_static);

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

  fprintf(report, "-------------------------------------------------\n\n");
  fclose(report);
  

  return 0;
}

void print_usage() {
  printf("Usage: zad2 length block_size alocation operation\n"
         "  length - length of table.\n"
         "  block_size - size of each block.\n"
         "  alocation - alocation type [s|d] s - static, d - dynamic.\n"
         "  operation - operation to execute. \n"
         "    * search value - search block of sum similar to value.\n"      
         "    * swap value - remove value blocks, then add new ones.\n"
         "    * alt_swap number - remove then add one block number times.\n");
}

void exec_and_save(FILE * f, Table * a, char * op, int arg) {
  Timing t;
  if (strcmp(op, "search") == 0) {
    fprintf(f, "Search for element with sum of element from index %d (%ld)\n", arg, block_sum(a, arg));
    t = get_timing();
    size_t i = search(a, arg);
    end_timing(&t);
    save_timing(f, &t);
    fprintf(f, "Element found on index: %ld, with sum %ld.\n\n", i, block_sum(a, i));
  } else if (strcmp(op, "swap") == 0) {
    fprintf(f, "Removing %d elements, then adding new ones\n", arg);
    t = get_timing();
    swap(a, arg);
    end_timing(&t);
    save_timing(f, &t);
  } else if (strcmp(op, "alt_swap") == 0) {
    fprintf(f, "Remove one element, then add new one %d times\n", arg);
    t = get_timing();
    alt_swap(a, arg);
    end_timing(&t);
    save_timing(f, &t);
  } else {
    fprintf(stderr, "Wrong operation.\n");
  }
}

Table * table_create(size_t length, size_t block_size, int is_static) {
  Table * a;
  a = create_table(length, block_size, is_static);

  for (size_t i = 0; i < a->length; ++i) {
    block_add(a, i);
  }

  return a;
}

void block_del(Table * a, size_t index) {
  delete_block(a, index);
}

void block_add(Table * a, size_t index) {
  char *  block = block_gen(a->block_size);
  add_block(a, index, block);
}

size_t search(Table * a, size_t sum) {
  return search_block(a, sum);
}

char * block_gen(size_t size) {
  char * block = calloc(size, sizeof(char));
  for (size_t i = 0; i < size; i++) {
    block[i] = rand() % 26 + 'a';
  }

  return block;
}

void swap(Table * a, size_t number) {
  for (size_t b = 0; b < number; b++) {
    block_del(a, b);
  }

  for (size_t b = 0; b < number; b++) {
    block_add(a, b);
  }
}

void alt_swap(Table * a, size_t number) {
  for (size_t b = 0; b < number; b++) {
    block_add(a, b);
  }
  
  for (size_t b = 0; b < number; b++) {
    block_del(a, 0);
  }
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
  fprintf(f, "  real: %.6f, user: %.6f, system: %.6f\n\n",
         (double) t->real / CLOCKS_PER_SEC,
         t->user.tv_sec + t->user.tv_usec / 10e6,
         t->system.tv_sec + t->system.tv_usec / 10e6);
}
