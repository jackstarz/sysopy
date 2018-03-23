#define _XOPEN_SOURCE     // strptime
#define _GNU_SOURCE       // lstat
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

int dates_compare(struct tm *, struct tm *);
void list(const char *, struct tm, char);
void print_usage();
void print_permissions(struct stat *);

int main(int argc, char *argv[]) {

  const char * date_format = "%d-%m-%Y";
  const char * path;
  char mode;
  struct tm tm = {0};

  if (argc < 2) {
    print_usage();
    return EXIT_FAILURE;
  }

  path = argv[1];

  if (argc < 3) {
    printf("No date mode specified\n");
    print_usage();
    return EXIT_FAILURE;
  }

  if (argc < 4) {
    fprintf(stderr, "No date specified.\n");
    print_usage();
    return EXIT_FAILURE;
  }

  strptime(argv[3], date_format, &tm);
  mode = argv[2][0];

  list(path, tm, mode);

  return 0;
}

void list(const char * path, struct tm other_date, char mode) {
  if (!path) {
    fprintf(stderr, "Incorrect path specified.\n");
    return;
  }

  DIR * dir = opendir(path);
  if (!dir) {
    fprintf(stderr, "Cannot open folder %s.\n", path);
    return;
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

    int date_diff = dates_compare(f_date, &other_date);

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
        list(path_long, other_date, mode);
      }
      content = readdir(dir);
    }
  }

  closedir(dir);
}

// 1 if t2 is earlier
// 0 if equal
// -1 if t2 is later
int dates_compare(struct tm * t1, struct tm * t2) {
  int year_diff = t2->tm_year - t1->tm_year;
  int month_diff = t2->tm_mon - t1->tm_mon;
  int day_diff = t2->tm_mday - t1->tm_mday;

  if (year_diff < 0) {
    return 1;
  } else if (year_diff > 0) {
    return -1;
  } else {
    if (month_diff < 0) {
      return 1;
    } else if (month_diff > 0) {
      return -1;
    } else {
      if (day_diff < 0) {
        return 1;
      } else if (day_diff > 0) {
        return -1;
      } else {
        return 0;
      }
    }
  }
}

void print_usage() {
  printf("Usage: program path mode date.\n"
            "  <path> - path to folder.\n"
            "  <mode> - date showing mode ('>', '=' or '<').\n"
            "  <date> - date in format dd-mm-YYYY.\n");
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