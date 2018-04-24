#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "ipc_utility.h"

int pub_queue, priv_queue;

void remove_queue(void);

int main(void) {
  key_t pub_key, priv_key;
  char *home = getenv("HOME");
  struct mymsg mymsg;
  int id;
  int quit = 0;
  char msg_buf[MSG_LEN];


  pub_key = ftok(home, PUB_Q);
  priv_key = ftok(home, getpid());

  if ((pub_queue = msgget(pub_key, 0666)) == -1) {
    perror("IPC error: pub msgget"); exit(EXIT_FAILURE);
  }

  if ((priv_queue = msgget(priv_key, 0666 | IPC_CREAT)) == -1) {
    perror("IPC error: priv msgget"); exit(EXIT_FAILURE);
  }

  // zeby server dowiedzial sie o istnieniu klienta
  snprintf(msg_buf, MSG_LEN, "%d", priv_key);
  send_msg(&outmsg, pub_queue, INIT, -1, msg_buf);

  // odpowiedz z ID
  read_msg(&inmsg, priv_key, INIT);

  id = inmsg.clientid;
  printf("my id is: % d\n", id);

  char cc[] = "XDDDDDDD";
  strncpy(msg_buf, cc, MSG_LEN);
  //send_msg(&outmsg, pub_queue, MIRROR, -1, msg_buf);

  puts("sent");

  //read_msg(&inmsg, priv_queue, MIRROR);

  atexit(remove_queue);
  exit(EXIT_SUCCESS);
}

void remove_queue() {
    msgctl(priv_queue, IPC_RMID, NULL);
}
