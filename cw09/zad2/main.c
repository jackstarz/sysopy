#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

typedef enum search_mode_t
{
  LT = '<',
  EQ = '=',
  GT = '>'
} SearchMode;

typedef struct config_t
{
  int         P;
  int         K;
  int         N;
  FILE        *file;
  int         L;
  SearchMode  search_mode;
  int         verbose;
  int         nk;
} Config;

typedef struct data_t
{
  char  **buf;
  int   p_pos;
  int   k_pos;
  int   lines_count;
} Data;

void  load_config(char *);
void  init_data();
void  create_threads();
void  *start_producing(void *);
void  *start_consuming(void *);
int   correct_length(size_t);
void  clean_up();
void  sig_handle(int);

Config      config;
Data        data;
pthread_t   *producers;
pthread_t   *consumers;
sem_t       buf_readable;
sem_t       buf_writeable;
sem_t       buf_mutex;

int
main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: ./zad1 <config_file>\n"); exit(1);
  }

  load_config(argv[1]);
  init_data();
  create_threads();
  
  for (int i = 0; i < config.P; ++i) 
  {
    pthread_join(producers[i], NULL);
  }
  
  while (1)
  {
    sem_wait(&buf_mutex);
    if (data.lines_count == 0) break;
    sem_post(&buf_mutex);
  }

  clean_up();

  return 0;
}

void
load_config(char *path)
{
  FILE  *config_file;
  int   read;
  char  attr_name[32], attr_value[32];

  if ((config_file = fopen(path, "r")) == NULL)
  {
    perror(path); exit(1);
  }

  while ((read = fscanf(config_file, "%s %s", attr_name, attr_value)) != EOF)
  {
    if (strcmp(attr_name, "P") == 0)
    {
      config.P = atoi(attr_value);
    }
    else if (strcmp(attr_name, "K") == 0)
    {
      config.K = atoi(attr_value);
    }
    else if (strcmp(attr_name, "N") == 0)
    {
      config.N = atoi(attr_value);
    }
    else if (strcmp(attr_name, "L") == 0)
    {
      config.L = atoi(attr_value);
    }
    else if (strcmp(attr_name, "SEARCH") == 0)
    {
      if (strcmp(attr_value, "<") == 0)
      {
        config.search_mode = LT;
      }
      else if (strcmp(attr_value, "=") == 0)
      {
        config.search_mode = EQ;
      }
      else if (strcmp(attr_value, ">") == 0)
      {
        config.search_mode = GT;
      }
    }
    else if (strcmp(attr_name, "FILE") == 0)
    {
      FILE *input_file;
      if ((input_file = fopen(attr_value, "r")) == NULL)
      {
        perror(attr_value); exit(1);
      }

      config.file = input_file;
    }
    else if (strcmp(attr_name, "VERBOSE") == 0)
    {
      config.verbose = atoi(attr_value);
    }
    else if (strcmp(attr_name, "NK") == 0)
    {
      config.nk = atoi(attr_value);
    }
  }

  fclose(config_file);
}

void
init_data()
{
  data.buf = malloc(config.N * sizeof(*data.buf));
  data.p_pos = 0;
  data.k_pos = 0;
  data.lines_count = 0;

  sem_init(&buf_mutex, 0, 1);
  sem_init(&buf_readable, 0, 0);
  sem_init(&buf_writeable, 0, config.N);
}

void
create_threads()
{
  producers = malloc(config.P * sizeof(*producers));
  consumers = malloc(config.K * sizeof(*consumers));

  signal(SIGINT, sig_handle);
  signal(SIGALRM, sig_handle);

  for (int i = 0; i < config.P; ++i)
  {
    pthread_create(&producers[i], NULL, &start_producing, NULL);
  }

  for (int i = 0; i < config.K; ++i)
  {
    pthread_create(&consumers[i], NULL, &start_consuming, NULL);
  }

  alarm(config.nk);
}

void
*start_producing(void *args)
{
  char    *line = NULL;
  size_t  line_size = 0;
  
  while (1)
  {
    sem_wait(&buf_writeable);
    sem_wait(&buf_mutex);

    if (getline(&line, &line_size, config.file) <= 0)
    {
      sem_post(&buf_mutex);
      break;
    }

    data.buf[data.p_pos] = malloc(line_size * sizeof(*data.buf[data.p_pos]));
    strcpy(data.buf[data.p_pos], line);

    if (config.verbose) {
      printf("P: placed line with length %ld at index %d\n", strlen(line), data.p_pos); 
    }

    data.p_pos = (data.p_pos + 1) % config.N;

    data.lines_count++;

    sem_post(&buf_readable);
    sem_post(&buf_mutex);
  }

  pthread_exit(NULL);
}

void
*start_consuming(void *args)
{
  char    *line;

  while (1)
  {
    sem_wait(&buf_readable);
    sem_wait(&buf_mutex);

    line = data.buf[data.k_pos];
    data.buf[data.k_pos] = NULL;

    if (correct_length(strlen(line)) != 0) {
      printf("C: found line with length %ld at index %d\n", strlen(line), data.k_pos); 
    }

    data.k_pos = (data.k_pos + 1) % config.N;

    data.lines_count--;

    sem_post(&buf_writeable);
    sem_post(&buf_mutex);

    free(line);
  }

  pthread_exit(NULL);
}

int
correct_length(size_t length)
{
  switch(config.search_mode)
  {
    case LT:
      return length < config.L;
    case EQ:
      return length == config.L;
    case GT:
      return length > config.L;
    default:
      return 0;
  }
}

void
sig_handle(int signo)
{
  for (int i = 0; i < config.P; ++i) {
    pthread_cancel(producers[i]);
  }

  for (int i = 0; i < config.K; ++i) {
    pthread_cancel(consumers[i]);
  }

  clean_up();
  exit(0);
}

void
clean_up()
{
  fclose(config.file);
  free(producers);
  free(consumers);

  for (int i = 0; i < config.N; ++i) {
    free(data.buf[i]);
  }
  free(data.buf);

  sem_destroy(&buf_mutex);
  sem_destroy(&buf_writeable);
  sem_destroy(&buf_readable);
}