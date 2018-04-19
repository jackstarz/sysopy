#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

void usr1_handler(int);

int
main(void)
{
  signal(SIGUSR1, usr1_handler);
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);

  srand(time(NULL) * getpid());
  time_t slp = rand() % 11;

  printf("pid: %d, sleeping for %lds\n", getpid(), slp);
  fflush(stdout);
  sleep(slp);

  kill(getppid(), SIGUSR1);
  sigsuspend(&mask);

  return slp;
}

void
usr1_handler(int signum)
{
  kill(getppid(), SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN)));
}