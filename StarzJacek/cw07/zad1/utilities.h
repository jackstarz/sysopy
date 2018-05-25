#ifndef UTILITIES_H
#define UTILITIES_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BARBER_PATH     getenv("HOME")
#define BARBER_ID       0x1337
#define MAX_QUEUE_SIZE  128

typedef enum barber_state_t
{
  SLEEPING,
  AWAKE,
  READY,
  FREE,
  BUSY
} BarberState;

typedef enum client_state_t
{
  NEW,
  INVITED,
  DONE
} ClientState;

typedef struct barberhop_t
{
  BarberState   barber_state;
  int           clients_count;
  int           chairs_count;
  pid_t         current_client;
  pid_t         clients_queue[MAX_QUEUE_SIZE];
} Barbershop;

long
get_time()
{
  struct timespec tms;
  clock_gettime(CLOCK_MONOTONIC, &tms);
  return tms.tv_nsec / 1000;
}

int
queue_empty(Barbershop *bs)
{
  return bs->clients_count == 0;
}

int
queue_full(Barbershop *bs)
{
  return bs->clients_count == bs->chairs_count;
}

void
enqueue(Barbershop *bs, pid_t client)
{
  bs->clients_queue[bs->clients_count] = client;
  ++bs->clients_count;
}

void
dequeue(Barbershop *bs)
{
  for (int i = 0; i < bs->clients_count - 1; ++i)
  {
    bs->clients_queue[i] = bs->clients_queue[i + 1];
  }

  bs->clients_queue[bs->clients_count - 1] = 0;
  --bs->clients_count;
}

void
lock_semaphore(int sem_id)
{
  struct sembuf sem;
  sem.sem_num = 0;
  sem.sem_op = -1;
  sem.sem_flg = 0;

  if (semop(sem_id, &sem, 1) == -1)
  {
    perror("IPC error: semop (lock)"); exit(1);
  }
}

void
unlock_semaphore(int sem_id)
{
  struct sembuf sem;
  sem.sem_num = 0;
  sem.sem_op = 1;
  sem.sem_flg = 0;

  if (semop(sem_id, &sem, 1) == -1)
  {
    perror("IPC error: semop (unlock)"); exit(1);
  }
}

#endif