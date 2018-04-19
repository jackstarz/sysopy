#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "ipc_utility.h"

int send_msg(int, long);

int main(void) {
  key_t key;
  int msqid;
  char *home = getenv("HOME");

  if ((key = ftok(home, MAIN_Q)) == (key_t) -1) {
    perror("IPC error: ftok"); exit(EXIT_FAILURE);
  }

  if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
    perror("IPC error: msgget"); exit(EXIT_FAILURE);
  }

  send_msg(msqid, key);

  exit(EXIT_SUCCESS);
}

int send_msg(int msqid, long type) {
  pid_t pid = getpid();
  struct msgbuf mymsg;
  mymsg.mtype = type;
  snprintf(mymsg.mtext, MSG_LEN, "%d", pid);

  return msgsnd(msqid, &mymsg, sizeof(mymsg), 0);
}