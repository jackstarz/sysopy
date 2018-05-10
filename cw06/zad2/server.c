#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <mqueue.h>

#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "ipc_utility.h"

int clients[CLIENTS_LIMIT];

int pub_queue, priv_queue;

void remove_queue(void);
void sigint_handler(int);
void strrev(char *);

int main(void) {
  int id = 0;
  int quit = 0;
  mymsg msg;
  char *time_str;
  time_t curr_time;
  char client_name[20];
  
  struct mq_attr queue_attr;
  queue_attr.mq_maxmsg = QUEUE_SIZE;
  queue_attr.mq_msgsize = msgsz;
  
  if ((pub_queue = mq_open(server_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &queue_attr)) == -1) {
    perror("public queue");
    exit(EXIT_FAILURE);
  }

  printf("Server is running...\n\n");

  memset(clients, 0, CLIENTS_LIMIT * sizeof(int));  
  signal(SIGINT, sigint_handler);
  atexit(remove_queue);

  while (1) {
    if (quit == 1) {
      mq_getattr(pub_queue, &queue_attr);
      if (queue_attr.mq_curmsgs == 0) {
        break;
      }
    }

    mq_receive(pub_queue, (char*) &msg, msgsz, NULL);

    switch(msg.type) {
      case INIT:
        if (id >= CLIENTS_LIMIT) {
          fprintf(stderr, "Server is full.\n");
          continue;
        }
        printf("INIT request from: %d\n", msg.clientpid);
        sprintf(client_name, "/%d", msg.clientpid);

        if ((priv_queue = mq_open(client_name, O_WRONLY)) == -1) {
          perror("priv queue");
          exit(EXIT_FAILURE);
        }

        clients[id] = priv_queue;
        msg.clientid = id;
        printf("sending ID with id = %d\n", id);

        id++;
        break;

      case MIRROR:
        printf("MIRROR request from: %d\n", msg.clientpid);
        strrev(msg.text);
        break;

      case CALC:
        printf("CALC request from: %d\n", msg.clientpid);

        char *operation = strtok(msg.text, " ");
        char *s1 = strtok(NULL, " ");
        char *s2 = strtok(NULL, " ");

        if (operation == NULL || s2 == NULL || s2 == NULL) {
          printf("Error while parsing arguments.\n");
          strcpy(msg.text, "ERR");
          continue;
        }

        int a = atoi(s1);
        int b = atoi(s2);

        int result;

        if (strcmp(operation, "ADD") == 0) {
          result = a + b;
        } else if (strcmp(operation, "SUB") == 0) {
          result = a - b;
        } else if (strcmp(operation, "DIV") == 0) {
          result = a / b;
        } else if (strcmp(operation, "MUL") == 0) {
          result = a * b;
        } else {
          printf("Unknown operation.\n");
          strcpy(msg.text, "ERR");
        }

        snprintf(msg.text, MSG_LEN, "%d", result);
        break;

      case TIME:
        printf("TIME request from: %d\n", msg.clientpid);
        curr_time = time(NULL);
        time_str = ctime(&curr_time);
        if (time_str == NULL) {
          fprintf(stderr, "Failed to create time string.\n");
        } else {
          time_str[strcspn(time_str, "\n")] = 0;
          strncpy(msg.text, time_str, MSG_LEN);
        }
        break;

      case END:
        quit = 1;
        break;
    }

    mq_send(priv_queue, (char*) &msg, msgsz, 1);
  }

  exit(EXIT_SUCCESS);
}

void remove_queue() {
    mq_close(pub_queue);
    mq_unlink("/server");
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