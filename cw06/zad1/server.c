#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
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
  key_t pub_key, priv_key;
  char *home = getenv("HOME");
  mymsg msg;
  char *time_str;
  time_t curr_time;
  int quit = 0;
  
  pub_key = ftok(home, PUB_Q);

  if ((pub_queue = msgget(pub_key, 0666 | IPC_CREAT)) == -1) {
    perror("public queue");
    exit(EXIT_FAILURE);
  }

  printf("Server is running...\n\n");

  memset(clients, 0, CLIENTS_LIMIT * sizeof(int));  
  signal(SIGINT, sigint_handler);
  atexit(remove_queue);

  while (msgrcv(pub_queue, &msg, msgsz, 0, (quit ? IPC_NOWAIT : 0)) >= 0) {

    switch(msg.type) {
      case INIT:
        if (id >= CLIENTS_LIMIT) {
          fprintf(stderr, "Server is full.\n");
          continue;
        }
        printf("INIT request from: %d\n", msg.clientpid);
        priv_key = atoi(msg.text);
        
        if ((priv_queue = msgget(priv_key, 0666)) == -1) {
          perror("MSGGET"); exit(EXIT_FAILURE);
        }

        clients[id] = priv_queue;
        msg.clientid = id;
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
          strcpy(msg.text, "Parsing error");
          msgsnd(clients[msg.clientid], &msg, msgsz, 0);
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
          strcpy(msg.text, "Unknown operation");
          msgsnd(clients[msg.clientid], &msg, msgsz, 0);
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

    msgsnd(clients[msg.clientid], &msg, msgsz, 0);
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