#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
void usage(char * pname) {
    fprintf(stderr, "USAGE:%s -o Name -p Path\n", pname);
    exit(EXIT_FAILURE);
}
void CreateListing(char * p, FILE * o,const char * a) {
  if (chdir(p)) { perror(p); return; }
  DIR * dirp;
  struct dirent * dp;
  struct stat fs;

  if ((dirp = opendir("."))==NULL) {perror("opendir"); chdir(a); return;}
  do {
    errno = 0;
    if ((dp = readdir(dirp)) != NULL) {
      if (lstat(dp->d_name, &fs) != 0) ERR("lstat");
      fprintf(o, "%s %lld\n", dp->d_name,(long long)fs.st_size);

    }
  } while (dp != NULL);

  if (errno != 0) ERR("readdir");
  if (closedir(dirp) != 0) ERR("closedir");
  if (chdir(a)) ERR("chdir");
}
int main(int argc, char**argv) {
  char curPath[PATH_MAX];
  if (getcwd(curPath, PATH_MAX) == NULL) ERR("getcwd");

  char **paths = malloc(sizeof(char *) * argc);
  if (paths == NULL) ERR("malloc");
  int n = 0;
  char *out = NULL;
  int c;

  while ((c = getopt(argc, argv, "p:o:")) != -1) {

    switch (c) {
    case 'p': paths[n] = optarg; n++; break;
    case 'o':
      if (out != NULL) usage(argv[0]);
      out = optarg;
      break;
    default: usage(argv[0]); break;
    }
  }
  if (optind < argc) usage(argv[0]);

  FILE *f = stdout;
  if (out != NULL && (f = fopen(out, "w")) == NULL) ERR("fopen");

  for (int i = 0; i < n; i++)
    CreateListing(paths[i], f, curPath);

  if (f != stdout && fclose(f)) ERR("fclose");
  free(paths);
  return EXIT_SUCCESS;
}