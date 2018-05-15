#include <stdio.h>
#include <stdlib.h>

#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "utilities.h"

int main()
{


  key_t barber_key;

  if ((barber_key = ftok(BARBER_PATH, BARBER_ID)) == (key_t) -1) {
    perror("IPC error: ftok"); exit(1);
  }


}