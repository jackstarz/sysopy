#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 512

FILE *input = NULL;

void print_usage();
char **string_split(char *, char *);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  const char *path = argv[1];
  input = fopen(path, "r");
  if (!input) {
    fprintf(stderr, "Failed to open %s\n", path);
    exit(EXIT_FAILURE);
  }

  int pipefd[2][2];
  char line[BUF_SIZE];

  while (fgets(line, BUF_SIZE, input)) {
    char **commands = string_split(line, "|");
    int n = 0;
    int k = 0;

    while (commands[n] != NULL) {
      ++n;
    }

    for (k = 0; k < n; ++k) {
      // jesli jest co pipe'owac
      if (k > 1) {
        close(pipefd[k % 2][0]);
        close(pipefd[k % 2][1]);
      }

      pipe(pipefd[k % 2]);
      pid_t child = fork();

      if (child == 0) {
        char **args = string_split(commands[k], " \n\t");
        if (k != n - 1) {
          close(pipefd[k % 2][0]);
          dup2(pipefd[k % 2][1], STDOUT_FILENO);
        }
        if (k != 0) {
          close(pipefd[(k + 1) % 2][1]);
          dup2(pipefd[(k + 1) % 2][0], STDIN_FILENO);
        }
        execvp(args[0], args);
      }
    } 
    
    close(pipefd[k % 2][0]);
    close(pipefd[k % 2][1]);
  
    // loops until there are no more children
    while(wait(NULL) > 0) {};
  }

  fclose(input);
  exit(EXIT_SUCCESS);
}

void print_usage() {
  printf("Usage: ./zad2 <file>\n"
         "  <file> - path to file to fetch commands from.\n");
}

char **string_split(char *s, char *delim) {
  size_t count = 0;
  char *p = strtok(s, delim);
  char **words = NULL;  

  while (p) {
    words = realloc(words, sizeof(*words) * (++count));
    words[count - 1] = p;
    p = strtok(NULL, delim);
  }

  words = realloc(words, sizeof(*words) * (++count));
  words[count - 1] = NULL;

  return words;
}