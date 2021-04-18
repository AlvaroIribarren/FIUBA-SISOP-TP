#include <stdio.h>
#include <zconf.h>
#include <stdio.h>
#include <zconf.h>
#include <stdlib.h>
#include <wait.h>

#define READ 0
#define WRITE 1

void primes(int* pipe_izq, int maxNumber);

void primes(int* pipe_izq, int maxNumber){
    int prime;
    if (read(pipe_izq[READ], &prime, sizeof(int)) <= 0){
        printf("No hay mas numeros\n");
        exit(0);
    }

    printf("prime: %d\n", prime);

    int pipe_der[2];
    if (pipe(pipe_der) == -1){
        fprintf(stderr, "Error al crear pipe");
    }

    int actualPid = fork();

    int n = 0;
    if (actualPid > 0) {
        while(read(pipe_izq[READ], &n, sizeof(int)) > 0){
            if((n % prime) !=0){
                int res = write(pipe_der[WRITE], &n, sizeof(int));
                if (res == -1)
                    fprintf(stderr, "Error al escribir en el pipe");
            }
        }
        close(pipe_der[WRITE]);
        wait(NULL);
    } else { //hijo
        close(pipe_der[WRITE]);
        primes(pipe_der, maxNumber);
        exit(0);
    }
}

int main(int argc, char* argv[]){
    if (argc <= 1){
        fprintf(stderr, "Error, se necesita un segundo argumento el cual"
                        "representa el maximo numero primo a buscar.");
        exit(0);
    }
    char* num = argv[1];
    int maxNumber = atoi(num);
    printf("Num ingresado: %d\n", maxNumber);
    int pipeFd[2];
    if (pipe(pipeFd)){
        fprintf(stderr, "Error al crear el pipe");
    }

    int actualPid = fork();
    if (actualPid > 0){ //padre
        for(int i=2; i <= maxNumber; i++){
            int res = write(pipeFd[WRITE], &i, sizeof(int));
            if (res == -1)
                fprintf(stderr, "Error al escribir en el pipe");
        }
        close(pipeFd[WRITE]);
        wait(NULL);
    } else {
        close(pipeFd[WRITE]);
        primes(pipeFd, maxNumber);
        exit(0);
    }
    return 0;
}

