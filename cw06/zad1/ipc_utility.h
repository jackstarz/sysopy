#ifndef IPC_UTILITY_H
#define IPC_UTILITY_H

#define PUB_Q 'a'

#define MSG_LEN 64

typedef struct mymsg {
  long  type;
  int   clientid;
  char  text[MSG_LEN];
} mymsg;

const int  msgsz = sizeof(mymsg) - sizeof(long);

int send_msg(struct mymsg*, int, long, int, char *);

enum Jobs {
  MIRROR, CALC, TIME, END, INIT, ID
};
/*
int send_msg(struct mymsg *mymsg, int qid, long type, int clientid, char *text) {
  int result;

  mymsg->type = type;
  mymsg->clientid = clientid;
  strncpy(mymsg->text, text, MSG_LEN);

  if ((result = msgsnd(qid, mymsg, msgsz, 0)) == -1) {
    perror("msgsnd");
    return(EXIT_FAILURE); 
  }

  return result;
}

int read_msg(struct mymsg *mymsg, int qid, long type) {
  int result;

  if ((result = msgrcv(qid, mymsg, msgsz, type, 0)) == -1) {
    perror("msgrcv");
    return(EXIT_FAILURE); 
  };

  return result;
}
*/
#endif