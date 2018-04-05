#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define BUF_SIZE 512

typedef struct {
  struct timeval user;
  struct timeval system;
} Timing;

void print_usage();
Timing get_timing();
void end_timing(Timing *);
void save_timing(FILE *, Timing *);

int main(int argc, char *argv[]) {
  if (argc < 4) {
    print_usage();
    return(EXIT_FAILURE);
  }

  const char *path = argv[1];
  FILE *input = fopen(path, "r");
  if (!input) {
    fprintf(stderr, "Failed to open %s\n", path);
    return(EXIT_FAILURE);
  }

  rlim_t ptime = strtol(argv[2], NULL, 10);
  rlim_t vmem = strtol(argv[3], NULL, 10) * 1024 * 1024;  

  struct rlimit tlimit = { ptime, ptime };
  struct rlimit mlimit = { vmem, vmem };

  FILE *results = fopen("results.txt", "w");
  fprintf(results, "MAX CPU TIME: %ld s, MAX VIRTUAL MEMORY: %ld MiB.\n\n", ptime, vmem / 1024 / 1024);

  char line[BUF_SIZE];

  while (fgets(line, BUF_SIZE, input)) {
    char *command = strdupa(line);
    if (command[strlen(command) - 1] == '\n') {
      command[strlen(command) - 1] = '\0';
    }
    
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

    Timing t = get_timing();

    if (child == 0) {
      if (setrlimit(RLIMIT_CPU, &tlimit) != 0) {
        fprintf(stderr, "Failed to initalize CPU time limits\n");
        return(EXIT_FAILURE);
      }

      if(setrlimit(RLIMIT_AS, &mlimit) != 0) {
        fprintf(stderr, "Failed to initalize memory limits\n");
        return(EXIT_FAILURE);
      }

      execvp(args[0], args);
      exit(126);
    }

    wait(&status);
    if (status) {
      fprintf(stderr, "%s: failed to execute\n", args[0]);
      return(EXIT_FAILURE);
    }

    fprintf(results, "%s\n", command);
    end_timing(&t);
    save_timing(results, &t);
  }

  fclose(input);
  fclose(results);

  return(0);
}

void print_usage() {
  printf("Usage: ./zad3a <file> <processor limit> <memory limit>\n"
         "  <file>              - path to file to fetch commands from.\n"
         "  <processor limit>   - available processor time.\n"
         "  <memory limit>      - virtual memory size.\n");
}

Timing get_timing() {
  Timing t;
  struct rusage ru;

  getrusage(RUSAGE_CHILDREN, &ru);
  t.system = ru.ru_stime;
  t.user = ru.ru_utime;

  return t;
}

void end_timing(Timing * t) {
  struct rusage current_usage;
  getrusage(RUSAGE_CHILDREN, &current_usage);

  t->user.tv_sec = current_usage.ru_utime.tv_sec - t->user.tv_sec;
  t->user.tv_usec = current_usage.ru_utime.tv_usec - t->user.tv_usec;
  t->system.tv_sec = current_usage.ru_stime.tv_sec - t->system.tv_sec;
  t->system.tv_usec = current_usage.ru_stime.tv_usec - t->system.tv_usec;
}

void save_timing(FILE * f, Timing * t) {
  fprintf(f, "  user: %.6f s, system: %.6f s\n\n",
         t->user.tv_sec + t->user.tv_usec / 10e6,
         t->system.tv_sec + t->system.tv_usec / 10e6);
}