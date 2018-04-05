#include <stdlib.h>

int main(void) {
  size_t size = 50 * 1024 * 1024;
	int * memory = malloc(sizeof(int) * size);
  for (size_t i = 0; i < size; ++i) {
	  memory[i] = 1;
	}


	return 0;
}
