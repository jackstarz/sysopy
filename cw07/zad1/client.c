#include <stdio.h>
#include <unistd.h>

#include "utilities.h"

int main() {

  while(1) {
    print_time();
    sleep(1);
  }

  return 0;
}