#include <stdio.h>      // printf
#include <stdlib.h>     // atoi
#include <pthread.h>    // threads
#include <structs.h>    // router

// global variables declaration
router *routers = NULL; // ip/port of all the routers
router self_router;     // ip/port of instanced router

static void *udp_receiver(void *arg) {
    while(1) {

    }

    return NULL;
}

static void *udp_sender(void *arg) {
    while(1) {

    }

    return NULL;
}

static void *distance_vector(void *arg) {
    while (1) {

    }

    return NULL;
}

static void *routing(void *arg) {
    while (1) {

    }

    return NULL;
}

int get_routers_settings(int self_id){
    int flag = 1;
    router aux;
    FILE *routers_settings = fopen("config/roteador.config", "r");
        for(size_t i = 0, n_routers = 0; fscanf(routers_settings,"%d %d %s", &aux.id, &aux.port, aux.ip) != EOF; i++){
            routers = (router *) realloc(routers, sizeof(router) * ++(n_routers));

            routers[i].id = aux.id;
            routers[i].port = aux.port;
            sprintf((routers)[i].ip, "%s", aux.ip);

            if(aux.id == self_id){
                flag = 0;
                self_router = aux;
            }
        }
    fclose(routers_settings);

    if(flag){
        printf(">>[ERROR] Router with ID %d not configurated.\n", self_id);
        return 1;
    }

    return 0;
}

int main(int argc, char const *argv[]) {
    // get the id of the router from the parameters
    if(argc != 2) {
        printf(">>[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        return 1;
    }
    int self_id = atoi(argv[1]);

    // read the configs for the initial distance-vector
    if(get_routers_settings(self_id)){
        return 1;
    }

    // create all needed threads
    pthread_t udp_receiver_thread;
    pthread_t udp_sender_thread;
    pthread_t distance_vector_thread;
    pthread_t routing_thread;
    pthread_create(&udp_receiver_thread, NULL, udp_receiver, NULL);
    pthread_create(&udp_sender_thread, NULL, udp_sender, NULL);
    pthread_create(&distance_vector_thread, NULL, distance_vector, NULL);
    pthread_create(&routing_thread, NULL, routing, NULL);

    // terminal interface to create new messages
    while(1){

    }

    return 0;
}
