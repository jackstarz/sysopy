#include "utilities.h"

Barbershop  *bs;
int         shm_id;
sem_t       *sem_id;

void init(int);
void remove_ipcs();
void sig_handle();

int
main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: ./barber <chairs_count>\n"); exit(1);
  }

  int chairs_count;
  
  sscanf(argv[1], "%d", &chairs_count);

  init(chairs_count);

  unlock_semaphore(sem_id);

  while (1)
  {
    lock_semaphore(sem_id);

    switch(bs->barber_state)
    {
      case FREE:
        if (queue_empty(bs))
        {
          printf("%ld: barber fell asleep\n", get_time());
          bs->barber_state = SLEEPING;
        } else {
          pid_t current_client = bs->clients_queue[0];
          bs->current_client = current_client;
          printf("%ld: client %d invited\n", get_time(), current_client);
          bs->barber_state = READY;
        }
        break;

      case AWAKE:
        printf("%ld: barber woke up!\n", get_time());
        bs->barber_state = READY;
        break;

      case BUSY:
        printf("%ld: started cutting client %d\n", get_time(), bs->current_client);
        printf("%ld: finished cutting client %d\n", get_time(), bs->current_client);
        bs->current_client = 0;

        bs->barber_state = READY;
        break;

      default:
        break;
    }
    unlock_semaphore(sem_id);
  }

}

void
init(int chairs_count)
{
  atexit(&remove_ipcs);
  signal(SIGINT, sig_handle);
  signal(SIGTERM, sig_handle);

  if ((shm_id = shm_open(BARBER_PATH, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG)) == -1)
  {
    perror("IPC error: shm_open"); exit(1);
  }

  if (ftruncate(shm_id, sizeof(Barbershop)) ==  -1)
  {
    perror("IPC error: ftruncate"); exit(1);
  }

  if ((bs = mmap(NULL, sizeof(Barbershop), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == (void *) -1)
  {
    perror("IPC error: mmap"); exit(1);
  }

  if ((sem_id = sem_open(BARBER_PATH, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, 0)) == SEM_FAILED)
  {
    perror("IPC error: sem_open"); exit(1);
  }

  bs->barber_state = SLEEPING;
  bs->clients_count = 0;
  bs->chairs_count = chairs_count;
  bs->current_client = 0;
}

void
remove_ipcs()
{
  sem_close(sem_id);
  sem_unlink(BARBER_PATH);

  munmap(bs, sizeof(Barbershop));
  shm_unlink(BARBER_PATH);
}

void
sig_handle()
{
  remove_ipcs();
  exit(0);
}