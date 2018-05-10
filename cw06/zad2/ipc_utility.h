#ifndef IPC_UTILITY_H
#define IPC_UTILITY_H

#define MSG_LEN 64
#define CLIENTS_LIMIT 10
#define QUEUE_SIZE 9

const char server_path[] = "/server";

typedef struct mymsg {
  long  type;
  int   clientid;
  pid_t clientpid;
  char  text[MSG_LEN];
} mymsg;

const int msgsz = sizeof(mymsg);

enum Jobs {
  MIRROR = 1, TIME, END, INIT, ID, CALC
};

#endif