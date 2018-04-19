#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

size_t N;
size_t K;
pid_t *children;
pid_t *children_pending;
int alive = 0;
int pending = 0;

int get_child_pid(pid_t);
void remove_child(pid_t);
void int_handler(int, siginfo_t *, void *);
void usr1_handler(int, siginfo_t *, void *);
void chld_handler(int, siginfo_t *, void *);
void rt_handler(int, siginfo_t *, void *);

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("Usage: <N> <K>\n");
    exit(EXIT_FAILURE);
  }

  N = atoi(argv[1]);
  K = atoi(argv[2]);

  children = calloc(N, sizeof(pid_t));
  children_pending = calloc(N, sizeof(pid_t));

  struct sigaction action;

  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_SIGINFO;
  
  action.sa_sigaction = int_handler;
  sigaction(SIGINT, &action, NULL);

  action.sa_sigaction = usr1_handler;
  sigaction(SIGUSR1, &action, NULL);

  action.sa_sigaction = chld_handler;
  sigaction(SIGCHLD, &action, NULL);

  for (int i = SIGRTMIN; i <= SIGRTMAX; ++i) {
    action.sa_sigaction = rt_handler;
    sigaction(i, &action, NULL);
  }

  for (int i = 0; i < N; ++i) {
    pid_t child = fork();
    if (child == 0) {
      execl("./child", "./child", NULL);
      exit(EXIT_FAILURE);
    } else {
      children[alive++] = child;
    }
  }

  while(1) {};
}

int
get_child_pid(pid_t pid)
{
  for (int i = 0; i < N; ++i) {
    if (children[i] == pid) {
      return i;
    }
  }

  return -1;
}

void
remove_child(pid_t pid)
{
  for (int i = 0; i < N; ++i) {
    if (children[i] == pid) {
      children[i] = -1;
      return;
    }
  }
}

void
int_handler(int signum, siginfo_t *info, void *context)
{
  printf("SIGINT received.\n");

  for (int i = 0; i < N; ++i) {
    if (children[i] != -1) {
      kill(children[i], SIGKILL);
      waitpid(children[i], NULL, 0);
    }
  }

  exit(0);
}

void
usr1_handler(int signum, siginfo_t *info, void *context)
{
  printf("Parent: SIGUSR1 received. pid: %d.\n", info->si_pid);

  if (get_child_pid(info->si_pid) == -1) {
    return;
  }

  if (pending >= K) {
    printf("Parent: sending SIGUSR1 to child pid: %d.\n", info->si_pid);
    kill(info->si_pid, SIGUSR1);
    waitpid(info->si_pid, NULL, 0);
  } else {
    children_pending[pending++] = info->si_pid;
    if (pending >= K) {
      for (int i = 0; i < K; ++i) {
        if (children_pending[i] > 0) {
          printf("Parent: sending SIGUSR1 to child pid: %d.\n", children_pending[i]);
          kill(children_pending[i], SIGUSR1);
          waitpid(children_pending[i], NULL, 0);
        }
      }
    }
  }
}

void
chld_handler(int signum, siginfo_t *info, void *context)
{
  if (info->si_code == CLD_EXITED) {
    printf("Parent: child (pid): %d exited with status: %d.\n", info->si_pid, info->si_status);
  } else {
    printf("Parent: child (pid): %d exited by signal: %d.\n", info->si_pid, info->si_status);
  }
  alive--;
  if (alive == 0) {
    exit(0);
  }

  remove_child(info->si_status);
}

void
rt_handler(int signum, siginfo_t *info, void *context)
{
  printf("Parent: SIGRT received: SIGMIN+%i for pid: %d.\n", signum - SIGRTMIN, info->si_pid);
}