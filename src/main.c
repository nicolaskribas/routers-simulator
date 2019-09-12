#include <pthread.h>
#include "data_structures/structs.h"
#include "threads/writer.c"
#include "threads/sender.c"
#include "threads/receiver.c"
#include "data_structures/dijkstra.c"

int get_routers_settings(int *n_routers, router **routers, int self_id)
{
    int  i, flag = TRUE;
    router aux;
    FILE *routers_settings = fopen("../config/roteador.config", "r");
        for(i = 0, *n_routers = 0; fscanf(routers_settings,"%d %d %s", &aux.id, &aux.port, aux.ip) != EOF; i++)
        {
            *routers = realloc (*routers, sizeof(router) * ++(*n_routers));

            (*routers)[i].id = aux.id;
            (*routers)[i].port = aux.port;
            sprintf((*routers)[i].ip, "%s", aux.ip);

            if(aux.id == self_id) flag = FALSE;
        }
    fclose(routers_settings);

    if(flag){
        printf("[ERROR] Router with ID %d not configurated.\n", self_id);
        exit(0);
    }
    return(1);

    // int a, b, cost;
    // FILE *enlaces = fopen("../config/enlaces.config", "r");
    //     for(i = 0, n_edges = 0; fscanf(elaces,"%d %d %d", %a, %b, %cost) != EOF; i++)
    //     {
    //
    //
    //     }
    // fclose(enlaces);
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        exit(0);
    }
    int self_id = atoi(argv[1]);

    int n_routers;
    router *routers = NULL;

    get_routers_settings(&n_routers, &routers, self_id);

    pthread_t Receiver, Sender, Writer;

    pthread_create(&Receiver, NULL, receiver, NULL);
    pthread_create(&Sender, NULL, sender, NULL);
    pthread_create(&Writer, NULL, writer, NULL);

    pthread_join(Receiver, NULL);
    pthread_join(Sender, NULL);
    pthread_join(Writer, NULL);

    return(1);
}
