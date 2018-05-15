#ifndef IPC_UTILITY_H
#define IPC_UTILITY_H

#define PUB_Q 'a'
#define MSG_LEN 64
#define CLIENTS_LIMIT 10

typedef struct mymsg {
  long  type;
  int   clientid;
  pid_t clientpid;
  char  text[MSG_LEN];
} mymsg;

const int msgsz = sizeof(mymsg) - sizeof(long);

enum Jobs {
  MIRROR = 1, TIME, END, INIT, ID, CALC
};

#endif