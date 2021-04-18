#include <stdio.h>
#include <zconf.h>
#include <sys/wait.h>
#include "string.h"

#include <stdio.h>
#include <zconf.h>
#include <sys/wait.h>
#include "string.h"
#include <stdlib.h>

#ifndef NARGS
#define NARGS 4
#endif

#define MAX_LINE 1000

void launch_process(char** args);
int splitStringIntoArrayOfStrings(char* stringToSplit, char **arrayOfStrings);

int splitStringIntoArrayOfStrings(char* stringToSplit, char **arrayOfStrings){
    char *ptr;
    ptr = strtok (stringToSplit, " \n");
    int i = 0;
    while (ptr != NULL){
        i++;
        arrayOfStrings[i] = ptr;
        ptr = strtok (NULL, " \n");
    }
    return i + 1;
}

void launch_process(char** args){
    int r = fork();
    if (r == 0) {
        int err = execvp(args[0], args);
        if (err) {
            printf("Error\n");
        }
    }
    wait(NULL);
}

int main(int argc, char* argv[]){
    if (argc <= 1){
        fprintf(stderr, "Error, se necesita que comando ejecutar");
        exit(0);
    }
    char* args[NARGS + 2] = {NULL};
    char* command = argv[1];
    args[0] = command;
    char line[MAX_LINE];
    char* linePtr = line;
    memset(line, 0, MAX_LINE);
    size_t lineSize = MAX_LINE;
    int argsRead = 0;

    char str[MAX_LINE];
    memset(str, 0, MAX_LINE);

    while(argsRead < NARGS && getline(&linePtr, &lineSize, stdin) != -1) {
        strcat(str, linePtr);
        argsRead++;
    }

    int amountOfElements = splitStringIntoArrayOfStrings(str, args);
    args[amountOfElements] = NULL;
    launch_process(args);
    return 0;
}
