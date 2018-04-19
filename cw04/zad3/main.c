#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int L;
int Type;
pid_t child;

int parent_sent = 0;
int child_received = 0;
int parent_received = 0;

void print_values();

void chld_handler(int, siginfo_t *, void *);
void chld_process();
void prnt_handler(int, siginfo_t *, void *);
void prnt_process();

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "Usage: ./zad3 <L> <Type>\n");
    exit(EXIT_FAILURE);
  }

  L = atoi(argv[1]);
  Type = atoi(argv[2]);

  child = fork();

  if (child == 0) {
    chld_process();
  } else if (child > 0) {
    prnt_process();
  } else {
    exit(-1);
  }

  print_values();
}

void
chld_process()
{
  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = chld_handler;

  sigset_t sigs;
  sigfillset(&sigs);

  switch(Type) {
    case 1:
    case 2:
      sigdelset(&sigs, SIGUSR1);
      sigdelset(&sigs, SIGUSR2);
      sigaction(SIGUSR1, &action, NULL);
      sigaction(SIGUSR2, &action, NULL);
      break;

    case 3:
      sigdelset(&sigs, SIGRTMIN);
      sigdelset(&sigs, SIGRTMAX);

      sigaction(SIGRTMIN, &action, NULL);
      sigaction(SIGRTMAX, &action, NULL);
      break;
  }

  sigprocmask(SIG_SETMASK, &sigs, NULL);

  while(1) {
    sleep(1); 
  }
}

void
prnt_process()
{
  sleep(1);

  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = prnt_handler;

  sigaction(SIGINT, &action, NULL);

  switch(Type) {
    case 1:
    case 2:
      sigaction(SIGUSR1, &action, NULL);
      break;
    
    case 3:
      sigaction(SIGRTMIN, &action, NULL);
  }

  sigset_t sigs;  

  switch(Type) {
    case 1:
    case 2:
      sigfillset(&sigs);
      sigdelset(&sigs, SIGUSR1);
      sigdelset(&sigs, SIGINT);
      for (; parent_sent < L; ++parent_sent) {
        printf("Parent: sending SIGUSR1.\n");
        kill(child, SIGUSR1);
        if (Type == 2) sigsuspend(&sigs);
      }
      printf("Parent: sending SIGUSR2.\n");
      kill(child, SIGUSR2);
      break;

    case 3:
      for (; parent_sent < L; ++parent_sent) {
        printf("Parent: sending SIGRTMIN.\n");
        kill(child, SIGRTMIN);
      }
      ++parent_sent;
      printf("Parent: sending SIGRTMAX.\n");
      kill(child, SIGRTMAX);
      break;
  }

  int status = 0;
  waitpid(child, &status, 0);
  if (WIFEXITED(status)) {
    child_received = WIFEXITED(status);
  } else {
    exit(1);
  }
}

void
chld_handler(int signum, siginfo_t *info, void *context)
{
  if (info->si_pid != getppid()) {
    return;
  }

  switch(Type) {
    case 1:
    case 2:
      ++child_received;

      if (signum == SIGUSR1) {
        kill(getppid(), SIGUSR1);
        printf("Child: SIGUSR1 received and sent.\n");
      } else if (signum == SIGUSR2) {
        printf("Child: SIGUSR2 received.\n");
        printf("Signals received by child: %d.\n", child_received);
        exit(child_received);
      }
      break;

    case 3:
      ++child_received;

      if (signum == SIGRTMIN) {
        kill(getppid(), SIGRTMIN);
        printf("Child: SIGRTMIN received and sent.\n");
      } else if (signum == SIGRTMAX) {
        printf("Child: SIGRTMAX received.\n");
        printf("Signals received by child: %d.\n", child_received);
        exit(child_received);
      }
      break;
  }
}

void
prnt_handler(int signum, siginfo_t *info, void *context)
{
  if (signum == SIGINT) {
    printf("Parent: SIGINT received.\n");
    kill(child, SIGUSR2);
    print_values();
    exit(9);
  }

  if (info->si_pid != child) {
    return;
  }

  if ((Type == 1 || Type == 2) && signum == SIGUSR1) {
    ++parent_received;
    printf("Parent: SIGUSR1 received from child.\n");
  } else if (Type == 3 && signum == SIGRTMIN) {
    ++parent_received;
    printf("Parent: SIGRTMIN received from child.\n");
  }
}

void
print_values()
{
  printf("Signals sent from parent: %d\n", parent_sent);
  printf("Signals received by parent: %d\n", parent_received);
}