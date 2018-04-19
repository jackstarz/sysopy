#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "ipc_utility.h"

int id = 0;

int main(void) {
  key_t m_key, c_key;
  int msqid;
  char *home = getenv("HOME");
  struct msgbuf mymsg;

  m_key = ftok(home, MAIN_Q);
  c_key = ftok(home, CLIENT_Q);

  if ((msqid = msgget(m_key, 0666 | IPC_CREAT)) == -1) {
    perror("IPC error: msgget"); exit(EXIT_FAILURE);
  }

  while (1) {
    msgrcv(msqid, &mymsg, sizeof(mymsg), m_key, 0);
    printf("client nr %d, pid: %s.\n",id++, mymsg.mtext);
  }
  
  exit(EXIT_SUCCESS);
}