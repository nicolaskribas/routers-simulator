#include <stdio.h>      // printf()
#include <stdlib.h>     // atoi()
#include <pthread.h>    // threads
#include <structs.h>    // router
#include <settings.h>   // message len
#include <sys/socket.h> // socket()
#include <arpa/inet.h>  // IPPROTO_UDP
#include <semaphore.h>  //
// global variables
router *routers = NULL; // ip/port of all the routers
router self_router;     // ip/port of instanced router
int sock;               // socket used in the instanced router


pthread_mutex_t to_route_buf_mutex;
sem_t to_route_buf_full;
sem_t to_route_buf_empty;
message_packet to_route_buf[TO_ROUTE_BUF_LEN];  // buffer to store messages that need to be routed
int to_route_buf_rear = 0;

pthread_mutex_t to_send_buf_mutex;
sem_t to_send_buf_full;
sem_t to_send_buf_empty;
message_packet to_send_buf[TO_SEND_BUF_LEN];
int to_send_buf_rear = 0;

static void *udp_receiver(void *arg) {
    int recv_len;
    char buf[MAX_LEN];
    socklen_t addr_len;
    sockaddr_in origin_addr;

    while(1) {
        if((recv_len = recvfrom(sock, buf, MAX_LEN, 0, (sockaddr *) &origin_addr, &addr_len))) {
            printf(">>[ERROR] Can't receive packet.\n");
        }

        if(*(int *) buf == 2) {
            if(!sem_trywait(&to_route_buf_empty)) {
                pthread_mutex_lock(&to_route_buf_mutex);

                to_route_buf[to_route_buf_rear] = *(message_packet *) buf;
                to_route_buf_rear = (to_route_buf_rear + 1) % TO_ROUTE_BUF_LEN;

                pthread_mutex_unlock(&to_route_buf_mutex);

                sem_post(&to_route_buf_full);
            }else {
                printf(">>Package discarted, the buf is full\n");
            }
        }else {

        }
    }

    return NULL;
}

static void *udp_sender(void *arg) {
    int to_send_buf_front = 0;

    while(1) {
        sem_wait(&to_send_buf_full);
        pthread_mutex_lock(&to_send_buf_mutex);

        to_send_package = to_send_buf[to_send_buf_front];
        to_send_buf_front = (to_send_buf_front + 1) % TO_SEND_BUF_LEN;

        pthread_mutex_unlock(&to_send_buf_mutex);
        sem_post(&to_send_buf_empty);

        // envia
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
        for(size_t i = 0, n_routers = 0; fscanf(routers_settings,"%d %d %s", &aux.id, &aux.port, aux.ip) != EOF; i++) {
            routers = (router *) realloc(routers, sizeof(router) * ++(n_routers));

            routers[i].id = aux.id;
            routers[i].port = aux.port;
            sprintf((routers)[i].ip, "%s", aux.ip);

            if(aux.id == self_id) {
                flag = 0;
                self_router = aux;
            }
        }
    fclose(routers_settings);

    if(flag){
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

    // read the ip/port settings for all the routers
    if(get_routers_settings(self_id)) {
        printf(">>[ERROR] Router with ID %d not configurated.\n", self_id);
        return 1;
    }

    // create a socket
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
        printf(">>[ERROR] Can't create a socket.\n");
        return 1;
    }

    // bind socket
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(self_router.port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (sockaddr *) &addr, sizeof(addr))) {
        printf(">>[ERROR] Can't bind socket.\n");
        return 1;
    }

    //  initiate all semaphores
    sem_init(&to_route_buf_full, 0 , 0);
    sem_init(&to_route_buf_empty, 0, TO_ROUTE_BUF_LEN);

    sem_init(&to_send_buf_full, 0 , 0);
    sem_init(&to_send_buf_empty, 0, TO_SEND_BUF_LEN);

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
