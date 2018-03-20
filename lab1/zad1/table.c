#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define LENGTH 10000
#define BLOCK_SIZE 512

char static_table [LENGTH][BLOCK_SIZE];

Table * create_table(size_t length, size_t block_size, int is_static) {
  Table * table = (Table *) calloc(length, sizeof(Table));
  if (!table) {
    fprintf(stderr, "Table creating failed.\n");
    return NULL;
  }

  table->is_static = is_static;

  if (!is_static) {
      table->length = length;
      table->block_size = block_size;
      table->blocks = calloc(length, block_size);
  } else {
    table->length = LENGTH;
    table->block_size = BLOCK_SIZE;
  }

  return table;
}

void delete_table(Table * table) {
  if (!table->is_static) {
    for (size_t b = 0; b < table->length; b++) {
      free(table->blocks[b]);
      table->blocks[b] = NULL;
    }
    free(table);
    table = NULL;
  }
}

void add_block(Table * table, size_t index, char * block) {
  if (index >= table->length) {
    fprintf(stderr, "Index out of range.");
    return;
  }

  if (!table->is_static) {
    table->blocks[index] = calloc(table->block_size, table->block_size);
    if (!table->blocks[index]) {
      fprintf(stderr, "Creating block failed.");
      return;
    }
    strcpy(table->blocks[index], block);  
  } else {
    memcpy(static_table[index], block, table->block_size);
  }
}

void delete_block(Table * table, size_t index) {
  if (index >= table->length) {
    fprintf(stderr, "Index out of range.\n");
    return;
  }

  if (!table->is_static) {
    free(table->blocks[index]);
    table->blocks[index] = NULL;
  } else {
    // ??
    static_table[index][0] = 0;
  }

}

size_t block_sum(char * block) {
  size_t n = strlen(block);
  size_t sum = 0;
  for (size_t i = 0; i < n; i++) {
    sum += block[i];    
  }

  return sum;
}

// returns index of block with similar sum
size_t search_block(Table * table, size_t sum) {
  size_t diff = table->block_size * 'z'; 
  size_t index = 0;
  size_t temp_diff = 0;

  for (size_t b = 0; b < table->length; b++) {
    if (!table->is_static) {
      temp_diff = block_sum(table->blocks[b]) - sum;
    } else {
      temp_diff = block_sum(static_table[b]);
    }

    if (abs(temp_diff) < diff) {
      diff = temp_diff;
      index = b;
    }
  } 

  return index;
}