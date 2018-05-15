#ifndef UTILITIES_H
#define UTILITIES_H

#include <time.h>

#define BARBER_PATH   getenv("home")
#define BARBER_ID     0x1337

void print_time()
{
  struct timespec tms;
  clock_gettime(CLOCK_MONOTONIC, &tms);
  printf("%ld.%06ld\n", tms.tv_nsec, tms.tv_nsec / 1000);
}

#endif