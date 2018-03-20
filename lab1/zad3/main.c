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
void exec_and_save(FILE *, Table *, char *, int);
void block_del(Table *, size_t);
void block_add(Table *, size_t);
char * block_gen(size_t);
Table * table_create(size_t, size_t, int);

size_t search(Table *, size_t);
void change(Table *, size_t);
void alt_change(Table *, size_t);

Timing get_timing();
void end_timing(Timing *);
void save_timing(FILE *, Timing *);

void * handle;

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

  #ifdef DYNAMIC
    handle = dlopen("../zad1/table.so", RTLD_LAZY);
    if (!handle) {
      fprintf(stderr, "Failed to open dynamic library.\n");
      return -1;
    }
  #endif

  FILE * report;
  report = fopen("raport2.txt", "a");

  Timing t = get_timing();
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

  dlclose(handle);
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

void exec_and_save(FILE * f, Table * a, char * op, int arg) {
  Timing t;
  if (strcmp(op, "search") == 0) {
    fprintf(f, "\nSearch for element with sum: %d\n", arg);
    t = get_timing();
    size_t i = search(a, arg);
    end_timing(&t);
    save_timing(f, &t);
    fprintf(f, "Element found on index: %ld, with sum %ld.\n", i, /*block_sum(a->blocks[i]*/6);
  } else if (strcmp(op, "change") == 0) {
    fprintf(f, "\nRemove element from index: %d\n", arg);
    t = get_timing();
    change(a, arg);
    end_timing(&t);
    save_timing(f, &t);
  } else if (strcmp(op, "alt_change") == 0) {
    fprintf(f, "\nAdd element to index: %d\n", arg);
    t = get_timing();
    alt_change(a, arg);
    end_timing(&t);
    save_timing(f, &t);
  } else {
    fprintf(stderr, "Wrong operation.\n");
  }
}

Table * table_create(size_t length, size_t block_size, int is_static) {
  Table * a;
  #ifdef DYNAMIC
    Table * (*create_table)(size_t, size_t, int);
    create_table = (Table * (*) (size_t, size_t, int)) dlsym(handle, "create_table");
    a = (*create_table)(length, block_size, is_static);
  #endif
  
  #ifndef DYNAMIC
    a = create_table(length, block_size, is_static);
  #endif

  for (size_t i = 0; i < a->length; ++i) {
    block_add(a, i);
  }

  return a;
}

void block_del(Table * a, size_t index) {
  #ifdef DYNAMIC
    void (*delete_block)(Table *, size_t);
    delete_block = (void (*) (Table *, size_t)) dlsym(handle, "delete_block");
    (*delete_block)(a, index);
  #endif
  
  #ifndef DYNAMIC
    delete_block(a, index);
  #endif
}

void block_add(Table * a, size_t index) {
  char *  block = block_gen(a->block_size);
  #ifdef DYNAMIC
    void (*add_block)(Table *, size_t, char *);
    add_block = (void (*) (Table *, size_t, char *)) dlsym(handle, "add_block");
    (*add_block)(a, index, block);
  #endif
  
  #ifndef DYNAMIC
    add_block(a, index, block);
  #endif
}

size_t search(Table * a, size_t sum) {
  #ifdef DYNAMIC
    size_t (*search_block)(Table *, size_t);
    search_block = (size_t (*) (Table *, size_t)) dlsym(handle, "search_block");
    return (*search_block)(a, sum);
  #endif

  #ifndef DYNAMIC
    return search_block(a, sum);
  #endif
}

char * block_gen(size_t size) {
  char * block = calloc(size, sizeof(char));
  for (size_t i = 0; i < size; i++) {
    block[i] = rand() % ('z' - 'A') + 'A' + 1;
  }

  return block;
}

void change(Table * a, size_t number) {
  for (size_t b = 0; b < number; b++) {
    block_del(a, b);
  }

  for (size_t b = 0; b < number; b++) {
    block_add(a, b);
  }
}

void alt_change(Table * a, size_t number) {
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
  fprintf(f, "  real: %.6f, user: %.6f, system: %.6f\n",
         (double) t->real / CLOCKS_PER_SEC,
         t->user.tv_sec + t->user.tv_usec / 10e6,
         t->system.tv_sec + t->system.tv_usec / 10e6);
}
