#pragma once
#include <stddef.h>

typedef struct {
  char ** blocks;
  size_t length;
  size_t block_size;
  int is_static;
} Table;

Table * create_table(size_t, size_t, int);
void delete_table(Table *);
void add_block(Table *, size_t, char *);
void delete_block(Table *, size_t);
size_t block_sum(char *, size_t);
size_t search_block(Table *, size_t);