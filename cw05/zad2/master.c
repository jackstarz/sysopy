#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define BUF_SIZE 128

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "No path specified.\n");
    exit(EXIT_FAILURE);
  }

  if (mkfifo(argv[1], S_IRUSR | S_IWUSR) == -1) {
    fprintf(stderr, "Couldn't create pipe.\n");
    exit(EXIT_FAILURE);
  }

  char buf[BUF_SIZE];
  FILE *p = fopen(argv[1], "r");
  sleep(1);
  while (fgets(buf, BUF_SIZE, p) != NULL) {
    printf(buf);
  }

  fclose(p);
  exit(EXIT_SUCCESS);
}