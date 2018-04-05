#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 512

void print_usage();

int main(int argc, char *argv[]) {
  if (argc != 2) {
    print_usage();
    return(EXIT_FAILURE);
  }

  const char *path = argv[1];
  FILE *input = fopen(path, "r");
  if (!input) {
    fprintf(stderr, "Failed to open %s\n", path);
    return(EXIT_FAILURE);
  }

  char line[BUF_SIZE];

  while (fgets(line, BUF_SIZE, input)) {
    size_t argc = 0;
    char *p = strtok(line, " \n");
    char ** args = NULL;  

    while (p) {
      args = realloc(args, sizeof(*args) * (++argc));
      args[argc - 1] = p;
      p = strtok(NULL, " \n");
    }

    args = realloc(args, sizeof(*args) * (++argc));
    args[argc - 1] = NULL;

    pid_t child = fork();
    int status;

    if (child == 0) {
      execvp(args[0], args);
      exit(1);
    }

    wait(&status);
    if (status) {
      fprintf(stderr, "%s: failed to execute\n", args[0]);
      return(EXIT_FAILURE);
    }
  }

  fclose(input);

  return(EXIT_SUCCESS);
}

void print_usage() {
  printf("Usage: ./zad2 <file>\n"
         "  <file> - path to file to fetch commands from.\n");
}