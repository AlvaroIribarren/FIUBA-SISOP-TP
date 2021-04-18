#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int generateRandomBetweenZeroAndNine(void);

int generateRandomBetweenZeroAndNine(){
    int num = rand();
    return num;
}


int main(){
    srand(time(NULL));
    int pipePQ[2];
    int pipeQP[2];

    if (pipe(pipePQ) == -1) {
        perror("Fallo al crear pipe 1\n");
    }
    if (pipe(pipeQP) == -1) {
        perror("Fallo al crear pipe 2\n");
    }
    int pid = getpid();
    printf("Hola, soy PID %d: \n", pid);
    printf("pipe me devuelve: [%d, %d] \n", pipePQ[0], pipePQ[1]);
    printf("pipe me devuelve: [%d, %d] \n\n", pipeQP[0], pipeQP[1]);

    int actualPid = fork();

    if (actualPid > 0) { //soy padre
        printf("Donde fork me devuelve %d\n", actualPid);
        printf("  - getpid me devuelve: %d \n", getpid());
        printf("  - getppid me devuelve: %d \n", getppid());
        int num = generateRandomBetweenZeroAndNine();
        printf("  - random me devuelve: %d \n", num);
        printf("  - envio valor %d a traves de fd= %d \n\n", num, pipePQ[1]);
        if (write(pipePQ[1], &num, sizeof(int)) == -1){
            fprintf(stderr, "Error al enviar");
        }

        int newBuf;
        if (read(pipeQP[0], &newBuf, sizeof(int)) == -1){
            fprintf(stderr, "Error al leer");
        }
        printf("Hola, de nuevo PID %d\n", actualPid);
        printf("  - recibi valor %d via fd= %d \n", num, pipePQ[0]);
    } else { // soy hijo
        int childReceived;
        if (read(pipePQ[0], &childReceived, sizeof(int)) != -1){
            printf("Donde fork me devuelve %d\n", actualPid);
            printf("    - getpid me devuelve: %d \n", getpid());
            printf("    - getppid me devuelve: %d \n", getppid());
            printf("    - recibo valor %d via=%d \n", childReceived, pipeQP[0]);
            printf("    - reenvio valor en fd=%d y termino\n\n", pipeQP[1]);

            if (write(pipeQP[1], &childReceived, sizeof(int)) == -1){
                fprintf(stderr, "Error al enviar");
            }
        } else {
            fprintf(stderr, "Error al leer");
        }
    }
    return 0;
}


