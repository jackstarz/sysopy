#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define LENGTH 1000000
#define BLOCK_SIZE 1000

char static_table [LENGTH][BLOCK_SIZE];

Table * create_table(size_t length, size_t block_size, int is_static) {

  Table * table = (Table *) calloc(length, sizeof(Table));
  if (!table) {
    fprintf(stderr, "Table creating failed.\n");
    return NULL;
  }

  table->is_static = is_static;
  table->length = length;
  table->block_size = block_size;
  
  if (!is_static) {
    table->blocks = calloc(length, block_size);
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
  } else {
    for (size_t b = 0; b < table->length; b++) {
      static_table[b][0] = 0;
    }
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
    static_table[index][0] = 0;
  }

}

size_t block_sum(Table * table, size_t index) {
  size_t sum = 0;

  for (size_t i = 0; i < table->block_size; i++) {
    if (table->is_static) {
      sum += static_table[index][i];
    } else {
      sum += table->blocks[index][i];    
    }
  }

  return sum;
}

// returns index of block with similar sum
size_t search_block(Table * table, size_t index) {
  size_t diff = table->block_size * 'Z'; 
  size_t best_index = 0;
  size_t temp_diff = 0;
  size_t sum = block_sum(table, index);

  for (size_t b = 0; b < table->length; b++) {
    if (b == index) {
      continue;
    }

    if (!table->is_static) {
      temp_diff = block_sum(table, b) - sum;
    } else {
      temp_diff = block_sum(table, b) - sum;
    }

    if (abs(temp_diff) < diff) {
      diff = temp_diff;
      best_index = b;
    }
  } 

  return best_index;
}