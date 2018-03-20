#pragma once
#include <stddef.h>

typedef struct {
  char ** blocks;
  size_t length;
  size_t block_size;
  int is_static;
} Table;

Table * create_table(size_t length, size_t block_size, int is_static);
void delete_table(Table * table);
void add_block(Table * table, size_t index, char * block);
void delete_block(Table * table, size_t index);
size_t block_sum(char * block);
size_t search_block(Table * table, size_t char_sum);