#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

#define CASE_COMPARISON_STR "-i"

bool stringsAreEqual(char* str1, char* str2);
void getDirectories(int fd, char* basePath, char* query, bool caseComparison);

bool stringsAreEqual(char* str1, char* str2){
    return strcmp(str1, str2) == 0;
}

void getDirectories(int fd, char* basePath, char* query, bool caseComparison){
    struct dirent *dp;
    DIR* dir = fdopendir(fd);
    char actualPath[PATH_MAX];
    memset(actualPath, 0, PATH_MAX);

    while ((dp = readdir(dir)) != NULL) {
        if (!stringsAreEqual(dp->d_name, ".") && !stringsAreEqual(dp->d_name, "..")) {
            char* name = dp->d_name;

            bool empty = (basePath[0] == '\0');
            strcpy(actualPath, basePath);

            if (!empty) {
                strcat(actualPath, "/");
            }

            strcat(actualPath, name);
            if (caseComparison){
                if (strstr(name, query)){
                    printf("%s\n", actualPath);
                }
            } else {
                if (strcasestr(name, query)){
                    printf("%s\n", actualPath);
                }
            }

            int newFd = openat(fd, name, O_DIRECTORY);
            if (newFd != -1) {
                getDirectories(newFd, actualPath, query, caseComparison);
            }
        }
    }
    closedir(dir);
}


int main(int argc, char* argv[]){
    if (argc <= 1){
        fprintf(stderr, "Se necesita como argumento el path a buscar");
        exit(0);
    }
    char* str = argv[1];
    char path[PATH_MAX];
    memset(path, 0, sizeof(path));

    DIR* dir = opendir(".");
    if (dir == NULL){
        perror("An error ocurred in opening base directory");
    }
    int fd = dirfd(dir);
    char* query;

    if (stringsAreEqual(str, CASE_COMPARISON_STR)){
        query = argv[2];
        getDirectories(fd, path, query, false);
    } else {
        query = str;
        getDirectories(fd, path, query, true);
    }
    return 0;
}

