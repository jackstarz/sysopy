#ifndef UTILITIES_H
#define UTILITIES_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define BARBER_PATH     "/barber"
#define MAX_QUEUE_SIZE  128

typedef enum barber_state_t
{
  SLEEPING,
  AWAKE,
  READY,
  FREE,
  BUSY,
} BarberState;

typedef enum client_state_t
{
  NEW,
  INVITED,
  DONE,
} ClientState;

typedef struct barberhop_t
{
  BarberState   barber_state;
  int           head;
  int           tail;
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
  return (bs->head == -1) && (bs->tail == -1);
}

int
queue_full(Barbershop *bs)
{
  return bs->head == (bs->tail + 1) % bs->chairs_count;
}

void
enqueue(Barbershop *bs, pid_t client)
{
  if (queue_empty(bs))
  {
    bs->head = 0;
    bs->tail = 0;
  }
  else
  {
    bs->tail = (bs->tail + 1) & bs->chairs_count;
  }

  bs->clients_queue[bs->tail] = getpid();
}

void
dequeue(Barbershop *bs)
{
  if (bs->head == bs->tail)
  {
    bs->head = -1;
    bs->tail = -1;
  }
  else
  {
    bs->head = (bs->head + 1) % bs->chairs_count;
  }
}

void
take_semaphore(sem_t *sem_id)
{
  if (sem_wait(sem_id) == -1)
  {
    perror("IPC error: sem_wait"); exit(1);
  }
}

void
give_semaphore(sem_t *sem_id)
{
  if (sem_post(sem_id) == -1)
  {
    perror("IPC error: sem_post"); exit(1);
  }
}

#endif