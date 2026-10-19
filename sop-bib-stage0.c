#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdbool.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define NAME(x) #x

// join 2 path. returned pointer is for newly allocated memory and must be freed
char* join_paths(const char* path1, const char* path2)
{
    char* res;
    const int l1 = strlen(path1);
    if (path1[l1 - 1] == '/')
    {
        res = malloc(strlen(path1) + strlen(path2) + 1);
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
    }
    else
    {
        res = malloc(strlen(path1) + strlen(path2) + 2);  // additional space for "/"
        if (!res)
            ERR("malloc");
        strcpy(res, path1);
        res[l1] = '/';
        res[l1 + 1] = 0;
    }
    return strcat(res, path2);
}

void usage(int argc, char** argv)
{
    (void)argc;
    fprintf(stderr, "USAGE: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
}
char ** GetText(char * name) {
    FILE *f;
    if ((f=fopen(name,"r"))==NULL) ERR("fopen");
    if (fseek(f,0,SEEK_END)) ERR("fseek");
    long size = ftell(f);
    if (size<0) ERR("ftell");
    if (fseek(f,0,SEEK_SET)) ERR("fseek");

    char * line = malloc(size+2);                  // zmieści każdą linię pliku
    char ** tab = malloc((size+1)*sizeof(char*));  // linii nie może być więcej niż bajtów, +1 na NULL
    if (line==NULL || tab==NULL) ERR("malloc");

    int i=0;
    while (fgets(line,size+2,f)!=NULL){
        line[strcspn(line,"\n")]='\0';             // usuń '\n' z końca
        if ((tab[i]=strdup(line))==NULL) ERR("strdup");
        i++;
    }
    tab[i]=NULL;

    free(line);
    if (fclose(f)==EOF) ERR("fclose");
    return tab;
}

// zwraca kopię wartości pola (max 64 bajty) albo NULL, gdy pola nie ma; wynik trzeba zwolnić
char * GetField(char ** tab, const char * key) {
    for (int i=0; tab[i]!=NULL; i++) {
        if (strncmp(tab[i], key, strlen(key)) == 0) {
            char * value = strndup(tab[i] + strlen(key), 64);
            if (value==NULL) ERR("strndup");
            return value;
        }
    }
    return NULL;
}

void FreeText(char ** tab) {
    for (int i=0; tab[i]!=NULL; i++) free(tab[i]);
    free(tab);
}

void Missing(bool t) {
    if (!t) {
        printf("%s: missing!\n",NAME(t));
    }
}
void PrintInfo(char * path) {
    char ** tab = GetText(path);
    int i=0;
    bool hasAuthor = false;
    bool hasTitle = false;
    bool hasGenre = false;
    while (tab[i]!=NULL) {
        if (strncmp(tab[i], "author:", strlen("author:")) == 0){
            hasAuthor = true;
            fprintf(stdout,"%s\n",tab[i]);
        }
        else if (strncmp(tab[i], "title:", strlen("title:")) == 0){
            hasTitle = true;
            fprintf(stdout,"%s\n",tab[i]);
        }
        else if (strncmp(tab[i], "genre:", strlen("genre:")) == 0){
            hasGenre = true;
            fprintf(stdout,"%s\n",tab[i]);
        }
        free(tab[i]);
        i++;
    }
        Missing(hasAuthor);
        Missing(hasTitle);
        Missing(hasGenre);
}

void AddToIndexes(char * full) {
    char ** tab = GetText(full);
    char * title = GetField(tab, "title:");
    char * genre = GetField(tab, "genre:");
    FreeText(tab);

    if (title!=NULL) {
        char *target = join_paths("../..", full);
        char *link = join_paths("index/by-title", title);
        if (symlink(target, link)) ERR("symlink");
        free(target);
        free(link);

        if (genre!=NULL) {
            char *dir = join_paths("index/by-genre", genre);
            if (mkdir(dir,0777) && errno!=EEXIST) ERR("mkdir");
            target = join_paths("../../..", full);
            link = join_paths(dir, title);
            if (symlink(target, link)) ERR("symlink");
            free(target);
            free(link);
            free(dir);
        }
    }
    free(title);
    free(genre);
}

void SearchLibrary(char * path, const char * index) {
    DIR *dirp;
    if ((dirp = opendir(path))==NULL) ERR("opendir");
    struct dirent *dp;
    struct stat fs;
    do {
        errno = 0;
        if ((dp = readdir(dirp))!=NULL) {
            if (strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0) continue;
            char *full = join_paths(path, dp->d_name);
            if (lstat(full,&fs)) ERR("lstat");
            if (S_ISREG(fs.st_mode)) {
                char *target = join_paths("../..", full);
                char *link = join_paths(index, dp->d_name);
                if (symlink(target, link)) ERR("symlink");
                free(target);
                free(link);
                AddToIndexes(full);
            }
            if (S_ISDIR(fs.st_mode)) {
                SearchLibrary(full,index);
            }
            free(full);
        }
    } while (dp!=NULL);
    if (errno!=0) ERR("readdir");
    if (closedir(dirp)) ERR("closedir");
}

void MakeIndex(void) {
    if (mkdir("index",0777)) ERR("mkdir");
    if (mkdir("index/by-visible-title",0777)) ERR("mkdir");
    if (mkdir("index/by-title",0777)) ERR("mkdir");
    if (mkdir("index/by-genre",0777)) ERR("mkdir");
}

int main(void) {
    MakeIndex();
    SearchLibrary("library", "index/by-visible-title");
    return EXIT_SUCCESS;
}

