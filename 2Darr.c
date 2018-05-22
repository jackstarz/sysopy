#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void) {

  int in = open("infile", O_RDONLY);
  int out = open("outfile", O_WRONLY | O_CREAT);

  char buf[1024];
  int bytes_read;

  while ((bytes_read = read(in, buf, sizeof(buf))) > 0) {
    write(out, buf, bytes_read);
  }

  return 0;
}