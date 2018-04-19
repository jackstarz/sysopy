#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

int is_paused = 0;

void tstp_handler(int);
void int_handler(int); 
void print_time();

int
main(void) {
  struct sigaction a;

  a.sa_handler = tstp_handler;
  sigemptyset(&a.sa_mask);
  a.sa_flags = 0;

  sigaction(SIGTSTP, &a, NULL);
  signal(SIGINT, int_handler);

  while (1) {
    if (!is_paused) {
      print_time();
      sleep(1);
    }
  }

  return(EXIT_SUCCESS);
}

void
tstp_handler(int signo) {
  if (!is_paused) {

    is_paused = 1;
    printf("CTRL+Z - continue\n"
           "CTRL+C - exit\n");
  } else {
    is_paused = 0;
  }
}

void
int_handler(int signo) {
  printf("SIGINT received. Exiting now.\n");
  exit(EXIT_SUCCESS);
}

void
print_time() {
  struct tm* time_info;
  time_t seconds;
  char time_str[32];
  
  time(&seconds);
  time_info = localtime(&seconds);
  strftime(time_str, 32, "%H:%M:%S", time_info);
  printf("%s\n", time_str);
}