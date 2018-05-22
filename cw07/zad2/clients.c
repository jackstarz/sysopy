#include "utilities.h"

Barbershop  *bs;
ClientState client_state;
int         shm_id;
sem_t       *sem_id;

void        init();
int         run_client();
void        take_seat();
ClientState update_state();

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("Usage: ./clients <clients_count> <haircuts_count>"); exit(1);
  }

  int clients_count;
  int haircuts_count;

  sscanf(argv[1], "%d", &clients_count);
  sscanf(argv[2], "%d", &haircuts_count);

  init();

  for (int i = 0; i < clients_count; ++i)
  {
    if (!(fork()))
    {
      int haircuts = 0;
      while (haircuts < haircuts_count)
      {
        haircuts += run_client();
      }
      exit(0);
    }
  }

  while (wait(0)) if (errno != ECHILD) break;
}

void
init()
{
  if ((shm_id = shm_open(BARBER_PATH, O_RDWR, S_IRWXU | S_IRWXG)) == -1)
  {
    perror("IPC error: shm_open"); exit(1);
  }

  if (ftruncate(shm_id,  sizeof(Barbershop)) ==  -1)
  {
    perror("IPC error: ftruncate"); exit(1);
  }

  if ((bs = mmap(NULL, sizeof(Barbershop), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == (void *) -1)
  {
    perror("IPC error: mmap"); exit(1);
  }

  if ((sem_id = sem_open(BARBER_PATH, O_WRONLY, S_IRWXU | S_IRWXG, 0)) == SEM_FAILED)
  {
    perror("IPC error: sem_open"); exit(1);
  }
}

int
run_client()
{
  pid_t pid = getpid();
  client_state = NEW;

  take_semaphore(sem_id);

  if (bs->barber_state == SLEEPING)
  {
    printf("%ld: barber woke up by client %d\n", get_time(), pid);
    bs->barber_state = AWAKE;
    take_seat();
    bs->barber_state = BUSY;
  }
  else
  {
    if (!queue_full(bs))
    {
      enqueue(bs, pid);
      printf("%ld: client %d enetered the queue\n", get_time(), pid);
    }
    else
    {
      printf("%ld: client %d could not eneter the queue\n", get_time(), pid);
      give_semaphore(sem_id);
      return 0;
    }
  }

  give_semaphore(sem_id);

  while (client_state < INVITED)
  {
    take_semaphore(sem_id);
    if ((client_state = update_state()) == INVITED)
    {
      take_seat();
      bs->barber_state = BUSY;
    }
    give_semaphore(sem_id);
  }

  while (client_state < DONE)
  {
    take_semaphore(sem_id);
    if ((client_state = update_state()) == DONE)
    {
      printf("%ld: client %i shaved\n", get_time(), pid);
      bs->barber_state = FREE;
    }
    give_semaphore(sem_id);
  }

  return 1;  
}

void
take_seat()
{
  pid_t pid = getpid();

  if (client_state == INVITED)
  {
    dequeue(bs);
  }
  else if (client_state == NEW)
  {
    while (1)
    {
      give_semaphore(sem_id);
      take_semaphore(sem_id);
      if (bs->barber_state == READY)
      {
        break;
      }
    }
    client_state = INVITED;
    bs->current_client = pid;
  }
  printf("%ld: client %d took a seat\n", get_time(), pid);
}

ClientState
update_state()
{
  pid_t pid = getpid();

  if (bs->current_client == pid)
  {
    return INVITED;
  }

  if (client_state == INVITED && bs->current_client != pid)
  {
    return DONE;
  }

  return NEW;
}