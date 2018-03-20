#include <stdio.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

void list(const char *, const char *, char);

int main(int argc, char *argv[]) {

  char * date_format = "%d-%m-%Y";
  struct tm tm1 = {0};
  time_t t1;

  struct tm tm2 = {0};
  time_t t2;

  strptime(argv[1], date_format, &tm1);
  t1 = mktime(&tm1);

  strptime(argv[2], date_format, &tm2);
  t2 = mktime(&tm2);

  time_t diff = t2 - t1;

  if (diff < 0) {
    printf("First date is later.\n");
  } else if (diff == 0) {
    printf("Dates are equal.\n");
  } else {
    printf("First date is earilier.\n");
  }

  return 0;
}

void list(const char * path, const char * other_date, char mode) {
  DIR * dir = opendir(path);
  if (dir == NULL) {
    fprintf(stderr, "Cannot open folder %s.\n", path);
    return;
  }

  struct dirent * content = readdir(dir);
  struct stat st;
  char path_long[512];
  char * date_format = "%d-%m-%Y %H:%M";

  while (readdir(dir) != NULL) {
    strcpy(path_long, path);
    strcat(path_long, "/");
    strcat(path_long, content->d_name);
    stat(path_long, &st);

    char date_str[PATH_MAX];
    size_t date = strftime(date_str, PATH_MAX, date_format, localtime(&st.st_mtime));

    printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
    printf( (st.st_mode & S_IRUSR) ? "r" : "-");
    printf( (st.st_mode & S_IWUSR) ? "w" : "-");
    printf( (st.st_mode & S_IXUSR) ? "x" : "-");
    printf( (st.st_mode & S_IRGRP) ? "r" : "-");
    printf( (st.st_mode & S_IWGRP) ? "w" : "-");
    printf( (st.st_mode & S_IXGRP) ? "x" : "-");
    printf( (st.st_mode & S_IROTH) ? "r" : "-");
    printf( (st.st_mode & S_IWOTH) ? "w" : "-");
    printf( (st.st_mode & S_IXOTH) ? "x" : "-");


    printf(" %s ", date_str); 
    printf(" %s/%s %ld\n", path, content->d_name, st.st_size); 
    

    content = readdir(dir);
  }

  closedir(dir);
}