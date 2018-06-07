#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef unsigned char uint8;

uint8   *input_img;
uint8   *output_img;
double  *filter_values;
int     img_width;
int     img_height;
int     c;
char    magic[2];
int     max_value;
int     threads_count;

void    load_image(char *);
void    load_filter(char *);
double  filter_pixel(int, int);
void    *filter_image(void *);
void    save_image(char *);
long    get_time();
void    save_results(long);

int
main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("Usage: ./filter <threads_count> <input_image> <filter> <output_image>\n"); exit(1);
  }

  int   pixels_count;
  long  time_start;
  long  time_end;
  long  time_diff;

  load_image(argv[2]);
  load_filter(argv[3]);

  sscanf(argv[1], "%d", &threads_count);

  pixels_count = (int) round(img_width * img_height / threads_count);

  int args[threads_count][2];
  for (int i = 0; i < threads_count; ++i)
  {
    args[i][0] = i * pixels_count;
    args[i][1] = min(args[i][0] + pixels_count, img_width * img_height);
  }

  time_start = get_time();

  pthread_t threads[threads_count];
  for (int i = 0; i < threads_count; ++i)
  {
    pthread_create(threads + i, NULL, filter_image, args + i);
  }

  for (int i = 0; i < threads_count; ++i)
  {
    pthread_join(threads[i], NULL);
  }

  time_end = get_time();
  time_diff = time_end - time_start;

  save_results(time_diff);
  save_image(argv[4]);

  free(input_img);
  free(output_img);
  free(filter_values);

  return 0;
}

void
load_image(char *path)
{
  FILE *image;
  if ((image = fopen(path, "r")) == NULL)
  {
    perror(path); exit(1);
  }

  fscanf(image, "%s\n", magic);
  fscanf(image, "%d %d", &img_width, &img_height);
  fscanf(image, "%d", &max_value);

  input_img = malloc(img_width * img_height * sizeof(*input_img));
  output_img = malloc(img_width * img_height * sizeof(*output_img));

  for (int i = 0; i < img_height; ++i)
  {
    for (int j = 0; j < img_width; ++j)
    {
      fscanf(image, "%d", (int *) &input_img[i * img_width + j]);
    }
  }

  fclose(image);
}

void
load_filter(char *path)
{
  FILE *filter;

  if ((filter = fopen(path, "r")) == NULL)
  {
    perror(path); exit(1);
  }

  fscanf(filter, "%d", &c);

  filter_values = malloc(c * c * sizeof(*filter_values));

  for (int i = 0; i < c; ++i)
  {
    for (int j = 0; j < c; ++j)
    {
      fscanf(filter, "%lf", &filter_values[i * c + j]);
    }
  }

  fclose(filter);
}

double
filter_pixel(int x, int y)
{
  double  sum;
  int     a;
  int     b;

  sum = 0;
  for (int i = 0; i < c; ++i)
  {
    for (int j = 0; j < c; ++j)
    {
      a = min(img_height - 1, max(1, x - (int) ceil(c / 2) + i));
      b = min(img_width - 1, max(1, y - (int) ceil(c / 2) + j));

      sum += input_img[a * img_width + b] * filter_values[i * c + j];
    }
  }

  return sum;
}

void *
filter_image(void *arg_void)
{
  int x;
  int y;

  int * arg = (int *) arg_void;

  for (int i = *arg; i < *(arg + 1); ++i)
  {
    x = i / img_width;
    y = i % img_width;

    output_img[i] = (uint8) round(filter_pixel(x, y));
  }

  return NULL;
}

void
save_image(char *path)
{
  FILE *image;
  if ((image = fopen(path, "w")) == NULL)
  {
    perror(path); exit(1);
  }

  fprintf(image, "%s\n", magic);
  fprintf(image, "%d %d\n", img_width, img_height);
  fprintf(image, "%d\n", max_value);

  for (int i = 0; i < img_height; ++i)
  {
    for (int j = 0; j < img_width; ++j)
    {
      fprintf(image, "%d ", output_img[i * img_width + j]);
    }
    fprintf(image, "\n");
  }

  fclose(image);
}

long
get_time()
{
  struct timeval time;
  gettimeofday(&time, 0);
  long result = (long) time.tv_sec * 1000000 + (long) time.tv_usec;

  return result;
}

void
save_results(long time_diff)
{
  FILE *results;
  char *path = "Times.txt";

  if ((results = fopen(path, "a")) == NULL)
  {
    perror(path); exit(1);
  }

  fprintf(results, "Image size: %dx%d\n", img_width, img_height);
  fprintf(results, "Filter size: %d\n", c);
  fprintf(results, "Number of threads: %d\n", threads_count);
  fprintf(results, "processing time: %ld.%lds\n\n", time_diff / 1000000, time_diff % 1000000);

  fclose(results);
}