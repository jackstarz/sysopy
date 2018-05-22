#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int
main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: ./gen_filter <c>.\n"); exit(1);
  }

  int     c;
  double  *values;
  double  sum;
  FILE    *output_file;
  char    *path;

  sscanf(argv[1], "%d", &c);

  values = malloc(c * c * sizeof(*values));

  sum = 0;
  for (int i = 0; i < c; ++i)
  {
    for (int j = 0; j < c; ++j)
    {
      values[i * c + j] = rand() / (double) RAND_MAX;
      sum += values[i * c + j];
    }
  }

  asprintf(&path, "fltr_%d", c);

  output_file = fopen(path, "w");
  fprintf(output_file, "%d\n", c);

  for (int i = 0; i < c; ++i)
  {
    for (int j = 0; j < c; ++j)
    {
      values[i * c + j] /= sum;
      fprintf(output_file, "%lf ", values[i * c + j]);
    }
    fprintf(output_file, "\n");
  }

  free(values);
  fclose(output_file);

  return 0;
}