#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "ipc_utility.h"

int pub_queue, priv_queue;

void remove_queue(void);
void sigint_handler(int);

int main(void) {
  key_t pub_key, priv_key;
  pid_t pid = getpid();
  char *home = getenv("HOME");
  int id;
  char buf[MSG_LEN];
  mymsg msg;
  char *line_buf;
  size_t line_n = MSG_LEN;

  pub_key = ftok(home, PUB_Q);
  priv_key = ftok(home, pid);

  atexit(remove_queue);  
  signal(SIGINT, sigint_handler);

  if ((pub_queue = msgget(pub_key, 0666)) == -1) {
    perror("public queue");
    exit(EXIT_FAILURE);
  }

  if ((priv_queue = msgget(priv_key, 0666 | IPC_CREAT)) == -1) {
    perror("private queue");
    exit(EXIT_FAILURE);
  }

  snprintf(msg.text, MSG_LEN, "%d", priv_key);
  msg.type = INIT;
  msg.clientpid = pid;
  msgsnd(pub_queue, &msg, msgsz, 0);

  msgrcv(priv_queue, &msg, msgsz, INIT, 0);
  id = msg.clientid;

  while (1) {
    printf("> ");
    getline(&line_buf, &line_n, stdin);
    strcpy(buf, line_buf);            
    line_buf = strtok(line_buf, " \n");
    buf[strcspn(line_buf, "\n")] = 0;

    if (strcmp(buf, "MIRROR") == 0) {
      msg.type = MIRROR;
      
      if ((line_buf = strtok(NULL, " \n")) == NULL) {
        fprintf(stderr, "give me some string to mirror.\n");
        continue;
      }

      line_buf[strcspn(line_buf, "\n")] = 0;
      strncpy(msg.text, line_buf, MSG_LEN);
    } else if (strcmp(buf, "TIME") == 0) {
      msg.type = TIME;
    } else if (strcmp(buf, "CALC") == 0) {
      msg.type = CALC;

      if ((line_buf = strtok(NULL, "\n")) == NULL) {
        fprintf(stderr, "give me something to calculate.\n");
        continue;
      }

      line_buf[strcspn(line_buf, "\n")] = 0;
      strncpy(msg.text, line_buf, MSG_LEN);
    } else if (strcmp(buf, "END") == 0) {
      msg.type = END;
      msgsnd(pub_queue, &msg, msgsz, 0);
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Unknown command.\n");
      continue;
    }

    msg.clientid = id;
    msg.clientpid = pid;
    msgsnd(pub_queue, &msg, msgsz, 0);

    if (msgrcv(priv_queue, &msg, msgsz, msg.type, 0) > 0) {
      printf("response: %s\n", msg.text);
    }
  }

  exit(EXIT_SUCCESS);
}

void remove_queue() {
    msgctl(priv_queue, IPC_RMID, NULL);
}

void sigint_handler(int signum) {
  remove_queue();
  exit(EXIT_SUCCESS);
}