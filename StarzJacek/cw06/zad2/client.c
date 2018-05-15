#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>

#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ipc_utility.h"

int pub_queue, priv_queue;

void remove_queue(void);
void sigint_handler(int);

int main(void) {
  pid_t pid = getpid();
  int id;
  char buf[MSG_LEN];
  mymsg msg;
  char *line_buf;
  size_t line_n = MSG_LEN;
  char client_name[20];

  atexit(remove_queue);  
  signal(SIGINT, sigint_handler);

  if ((pub_queue = mq_open(server_path, O_WRONLY)) == -1) {
    perror("public queue");
    exit(EXIT_FAILURE);
  }

  struct mq_attr queue_attr;
  queue_attr.mq_maxmsg = QUEUE_SIZE;
  queue_attr.mq_msgsize = msgsz;

  sprintf(client_name, "/%d", pid);

  if ((priv_queue = mq_open(client_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr)) == -1) {
    perror("private queue");
    exit(EXIT_FAILURE);
  }

  msg.type = INIT;
  msg.clientpid = pid;
  if (mq_send(pub_queue, (char*) &msg, msgsz, 1) == -1) {
    printf("failed to init\n");
  }

  if (mq_receive(priv_queue, (char*) &msg, msgsz, NULL) == -1) {
    printf("Failed to get INIt response.\n");
  }

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
      mq_send(pub_queue, (char*) &msg, msgsz, 1);
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Unknown command.\n");
      continue;
    }

    msg.clientid = id;
    msg.clientpid = pid;

    mq_send(pub_queue, (char*) &msg, msgsz, 1);

    if (mq_receive(priv_queue, (char*) &msg, msgsz, NULL) != -1) {
      printf("response: %s\n", msg.text);
    }
  }

  exit(EXIT_SUCCESS);
}

void remove_queue() {
    mq_close(priv_queue);
    mq_unlink("/client");
}

void sigint_handler(int signum) {
  remove_queue();
  exit(EXIT_SUCCESS);
}