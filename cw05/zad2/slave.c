#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 128

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "No path and N specified.\n");
    exit(EXIT_FAILURE);
  }

  int p = open(argv[1], O_WRONLY);
  int N = atoi(argv[2]);

  pid_t pid = getpid();
  printf("slave pid: %d\n", pid);

  FILE *date;

  char date_buf[BUF_SIZE];
  char out_buf[BUF_SIZE];

  for (int i = 0; i < N; ++i) {
    date = popen("/bin/date", "r");
    fgets(date_buf, BUF_SIZE, date);
    pclose(date);

    sprintf(out_buf, "pid: %d, date: %s", pid, date_buf);
    write(p, out_buf, strlen(out_buf));
    sleep(rand() % 4 + 2);
  }

  close(p);
  exit(EXIT_SUCCESS);
} 