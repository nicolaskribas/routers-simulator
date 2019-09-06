#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct{
    int id;
    int port;
    char ip[15];
}routerSetings;

void *receiver (){

}
void *sender(){

}

void main(int argc, char *argv[]){

    if(argc != 2){
        printf("[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        exit(0);
    }

    int i, id, port, n_routers, self_id = *argv[1];
    char ip[15];
    routerSetings *routers;


    FILE *roteador = fopen("../config/roteador.config", "r");
        for(i = 0, n_routers = 0; fscanf(roteador,"%d %d %s", &id, &port, ip) != EOF; i++)
        {
            n_routers++;
            routers = (routerSetings *) realloc (routers, sizeof(routerSetings) * n_routers);

            routers[i].id = id;
            routers[i].port = port;
            sprintf(routers[i].ip, "%s", ip);
        }
    fclose(roteador);

    // int a, b, cost;
    // FILE *enlaces = fopen("../config/enlaces.config", "r");
    //     for(i = 0, n_edges = 0; fscanf(elaces,"%d %d %d", %a, %b, %cost) != EOF; i++)
    //     {
    //
    //
    //     }
    // fclose(enlaces);

    // pthread_t sender, receiver;
    // pthread_create(&sender,NULL, sender, NULL);
    // pthread_create(&receiver,NULL, receiver, NULL);
}
