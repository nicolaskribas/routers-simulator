#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        exit(0);
    }

    int id = atoi(argv[1]);

    FILE *roteador = fopen("../config/roteador.config", "r");

    fclose(roteador);

    FILE *enlaces = fopen("../config/enlaces.config", "r");

    fclose(enlaces);

    return 0;
}
