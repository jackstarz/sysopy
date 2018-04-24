#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "ipc_utility.h"

#define CLIENTS_LIMIT 5

int id = 0;
int clients[CLIENTS_LIMIT];

int pub_queue, priv_queue;


void remove_queue(void);
void sigint_handler(int);
void strrev(char *);

int main(void) {
  key_t pub_key, priv_key;
  char *home = getenv("HOME");
  char msg_buf[MSG_LEN];

  struct mymsg inmsg;
  struct mymsg outmsg;
  
  pub_key = ftok(home, PUB_Q);

  if ((pub_queue = msgget(pub_key, 0666 | IPC_CREAT)) == -1) {
    perror("IPC error: pub msgget"); exit(EXIT_FAILURE);
  }

  memset(clients, 0, CLIENTS_LIMIT * sizeof(int));  
  signal(SIGINT, sigint_handler);

  while (1) {
    int size = sizeof(inmsg) - sizeof(long);
    msgrcv(pub_queue, &inmsg, size, 0, 0);

    switch(inmsg.type) {
      case INIT:
        printf("got request from: %s\n", inmsg.text);
        priv_key = atoi(inmsg.text);
        
        if ((priv_queue = msgget(priv_key, 0666)) == -1) {
          perror("IPC error: priv msgget"); exit(EXIT_FAILURE);
        }

        clients[id] = priv_queue;
        snprintf(msg_buf, MSG_LEN, "%d", id);
        printf("sending ID with id = %d\n", id);
        send_msg(&outmsg, priv_queue, INIT, id, msg_buf);
        id++;
        break;

      case MIRROR:
        snprintf(msg_buf, MSG_LEN, "%s", "mirror");
        send_msg(&outmsg, priv_queue, MIRROR, 0, msg_buf);
        break;
    }
  }

  exit(EXIT_SUCCESS);
}

void remove_queue(void) {
    msgctl(pub_queue, IPC_RMID, NULL);
}

void sigint_handler(int signum) {
  remove_queue();
  exit(EXIT_SUCCESS);
}

void strrev(char *str) {
  if (str) {
    char *end = str + strlen(str) - 1;

    while (str < end) {
      *str ^= *end;
      *end ^= *str;
      *str ^= *end;
      str++;
      end--;
    }
  }
}