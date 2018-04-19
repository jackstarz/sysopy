#define MAIN_Q 'a'
#define CLIENT_Q 'b'

#define MSG_LEN 128

struct msgbuf {
  long mtype;
  char mtext[MSG_LEN];
};

enum Jobs {
  MIRROR, CALC, TIME, END
};

