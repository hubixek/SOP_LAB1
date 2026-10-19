#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
void CreateListing(char * p, char * o) {
  DIR * dirp;
  struct dirent * dp;
  //struct stat fs;
  if ((dirp = opendir("."))==NULL) ERR("opendir");
  do {
    errno = 0;
    if ((dp = readdir(dirp)) != NULL) {
      fprintf(stdout, "%s\n", dp->d_name);
    }
  } while (dp != NULL);

  if (errno != 0) ERR("readdir");
  if (closedir(dirp) != 0) ERR("closedir");
}
int main() {
  CreateListing("AV","AC");
}