#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

static int is_paused = 0;
static pid_t child = 0;

void tstp_handler(int);
void int_handler(int);
void exec_script();

int
main(void) {
  struct sigaction action;

  action.sa_handler = tstp_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  sigaction(SIGTSTP, &action, NULL);
  signal(SIGINT, int_handler);

  exec_script();

  while (1) {
  }
}

void
tstp_handler(int signo) {
  if (!is_paused) {
    is_paused = 1;
    kill(child, SIGKILL);
    printf("CTRL+Z - continue\n"
           "CTRL+C - exit\n");
  } else {
    is_paused = 0;
    exec_script();
  }
}

void
int_handler(int signo) {
  printf("SIGINT received. Exiting now.\n");
  exit(EXIT_SUCCESS);
}

void
exec_script() {
  pid_t proc = fork();
  if (proc < 0) {
    fprintf(stderr, "Failed to fork.\n");
    exit(EXIT_FAILURE);
  } else if (proc != 0) {
    child = proc;
  } else {
    execl("./print_date.sh", "./print_date.sh", NULL);
    exit(EXIT_FAILURE);
  }
}
