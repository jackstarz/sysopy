#define _GNU_SOURCE       // lstat
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>


void print_usage();
int list(const char *, struct tm *, char);
void print_permissions(struct stat *);
int dates_compare(struct tm *, struct tm *);
void zero_time(struct tm *);

int main(int argc, char *argv[]) {
  const char * date_format = "%d-%m-%Y";
  const char * path;
  char mode;
  struct tm date;

  if (argc != 4) {
    print_usage();
    return(EXIT_FAILURE);
  }

  path = argv[1];
  mode = argv[2][0];
  strptime(argv[3], date_format, &date);

  list(path, &date, mode);

  return(EXIT_SUCCESS);
}

int list(const char * path, struct tm *other_date, char mode) {
  if (!path) {
    fprintf(stderr, "Incorrect path specified.\n");
    return(EXIT_FAILURE);
  }

  DIR * dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "Cannot open folder %s.\n", path);
    return(EXIT_FAILURE);
  }

  struct dirent * content = readdir(dir);
  struct stat st;
  char path_long[PATH_MAX];
  char * date_format = "%d-%m-%Y";
  struct tm * f_date = malloc(sizeof(struct tm));

  while (content != NULL) {
    strcpy(path_long, path);
    strcat(path_long, "/");
    strcat(path_long, content->d_name);
    lstat(path_long, &st);

    char date_str[32];
    f_date = localtime(&st.st_mtime);
    strftime(date_str, 32, date_format, localtime(&st.st_mtime));

    int date_diff = dates_compare(f_date, other_date);

    if (strcmp(content->d_name, "..") == 0 || strcmp(content->d_name, ".") == 0) {
      content = readdir(dir);
      continue;
    } else {
      if (S_ISREG(st.st_mode)) {
        if ((date_diff > 0 && mode == '>') || (date_diff == 0 && mode == '=') || (date_diff < 0 && mode == '<')) {
          print_permissions(&st);
          printf(" %s", date_str); 
          printf(" %s/%s %ld\n", path, content->d_name, st.st_size);
        }
      }

      if (S_ISDIR(st.st_mode)) {
        pid_t child = fork();
        if (child == 0) {
          list(path_long, other_date, mode);
          exit(0);
        }
      }
      content = readdir(dir);
    }
  }

  closedir(dir);
  return(EXIT_SUCCESS);
}

// 1 if t2 is earlier
// 0 if equal
// -1 if t2 is later
int dates_compare(struct tm * t1, struct tm * t2) {
  zero_time(t1);
  zero_time(t2);  

  double diff = difftime(mktime(t2), mktime(t1));
  if (diff < 0.) {
    return(1);
  } else if (diff == 0.) {
    return(0);
  } else {
    return(-1);
  }
}

void print_permissions(struct stat * st) {
  printf( (S_ISDIR(st->st_mode)) ? "d" : "-");
  printf( (st->st_mode & S_IRUSR) ? "r" : "-");
  printf( (st->st_mode & S_IWUSR) ? "w" : "-");
  printf( (st->st_mode & S_IXUSR) ? "x" : "-");
  printf( (st->st_mode & S_IRGRP) ? "r" : "-");
  printf( (st->st_mode & S_IWGRP) ? "w" : "-");
  printf( (st->st_mode & S_IXGRP) ? "x" : "-");
  printf( (st->st_mode & S_IROTH) ? "r" : "-");
  printf( (st->st_mode & S_IWOTH) ? "w" : "-");
  printf( (st->st_mode & S_IXOTH) ? "x" : "-");
}

void print_usage() {
  printf("Usage: program <path> <mode> <date>.\n"
            "  <path> - path to folder.\n"
            "  <mode> - date showing mode ('>', '=' or '<').\n"
            "  <date> - date in format dd-mm-YYYY.\n");
}

void zero_time(struct tm* date) {
  date->tm_hour = 0;
  date->tm_min = 0;
  date->tm_sec = 0;
} 
