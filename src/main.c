#include <stdio.h>      // printf()
#include <stdlib.h>     // atoi()
#include <pthread.h>    // threads
#include <structs.h>    // router
#include <settings.h>   // message len
#include <sys/socket.h> // socket()
#include <arpa/inet.h>  // IPPROTO_UDP
#include <semaphore.h>  //
#include <time.h>       // timespec
#include <string.h>     // memcpy

#define MSG_TYPE 0
#define ACK_TYPE 1
#define D_V_TYPE 2

// global variables

routing_row *routing_table = NULL;

router self_router;     // ip/port of instanced router
int sock;               // socket used in the instanced router
int current_seq_num = 0;

router *neighbors = NULL;
int n_neighbors = 0;
int n_routers = 0;

pthread_mutex_t self_d_v_mutex;
int *self_distance_vector[2];

pthread_mutex_t ack_mutex;

pthread_mutex_t to_send_buf_mutex;
sem_t to_send_buf_full;
sem_t to_send_buf_empty;
packet to_send_buf[TO_SEND_BUF_LEN];
int to_send_buf_rear = 0;

pthread_mutex_t d_v_buf_mutex;
sem_t d_v_buf_full;
sem_t d_v_buf_empty;
packet d_v_buf[D_V_BUF_LEN];
int d_v_buf_rear = 0;


static void *receiver(void *arg) {
    int recv_len;
    packet buf;
    socklen_t addr_len;
    struct sockaddr_in origin_addr;
    int old_origin;
    while(1) {
        if((recv_len = recvfrom(sock, &buf, sizeof(packet), 0, (struct sockaddr *) &origin_addr, &addr_len))) {
            printf(">>[ERROR] Can't receive packet.\n");
        }

        switch(buf.type) {
            case MSG_TYPE: // for message packets
                if(buf.id_destination == self_router.id) {
                    printf(">>New message from router %d: %s\n", buf.id_origin, (buf.content)+sizeof(int));
                    buf.type = ACK_TYPE;
                    old_origin = buf.id_origin;
                    buf.id_origin = buf.id_destination;
                    buf.id_destination = old_origin;
                }
                if(!sem_trywait(&to_send_buf_empty)) {
                    pthread_mutex_lock(&to_send_buf_mutex);

                    to_send_buf[to_send_buf_rear] = buf;
                    to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

                    pthread_mutex_unlock(&to_send_buf_mutex);

                    sem_post(&to_send_buf_full);
                }else {
                    printf(">>Package discarted because the buffer is full\n");
                }
            break;

            case ACK_TYPE: // for ack packets
                if(buf.id_destination == self_router.id) {
                    if(*(buf.content) == current_seq_num) {
                        pthread_mutex_unlock(&ack_mutex);
                    }
                }else {
                    if(!sem_trywait(&to_send_buf_empty)) {
                        pthread_mutex_lock(&to_send_buf_mutex);

                        to_send_buf[to_send_buf_rear] = buf;
                        to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

                        pthread_mutex_unlock(&to_send_buf_mutex);

                        sem_post(&to_send_buf_full);
                    }else {
                        printf(">>Package discarted because the buffer is full\n");
                    }
                }
            break;

            case D_V_TYPE: // for distance-vector packets
                if(!sem_trywait(&d_v_buf_empty)) {
                    pthread_mutex_lock(&d_v_buf_mutex);

                    d_v_buf[d_v_buf_rear] = buf;
                    d_v_buf_rear = (d_v_buf_rear + 1) % D_V_BUF_LEN;

                    pthread_mutex_unlock(&d_v_buf_mutex);

                    sem_post(&d_v_buf_full);
                }else {
                    printf(">>Package discarted because the buffer is full\n");
                }
            break;
        }
    }

    return NULL;
}

static void *sender(void *arg) {
    int to_send_buf_front = 0;

    packet to_send_packet;

    struct sockaddr_in dest_addr;
    int addr_len = sizeof(dest_addr);
    dest_addr.sin_family = AF_INET;

    int port;
    char ip[15];

    while(1) {
        sem_wait(&to_send_buf_full);
        pthread_mutex_lock(&to_send_buf_mutex);

        to_send_packet = to_send_buf[to_send_buf_front];
        to_send_buf_front = (to_send_buf_front + 1) % TO_SEND_BUF_LEN;

        pthread_mutex_unlock(&to_send_buf_mutex);
        sem_post(&to_send_buf_empty);

        // next_router = routers[self_distance_vector[1][to_send_packet.id_destination-1]];
        // strcpy(ip, next_router.ip);
        // port = next_router.port;

        if(inet_aton(ip, &dest_addr.sin_addr)) {
            printf(">>[ERROR] inet_aton()\n");
        }
        dest_addr.sin_port = htons(port);

        if(sendto(sock, &to_send_packet, sizeof(packet), 0, (struct sockaddr *) &dest_addr, addr_len)) {
            printf(">>[ERROR] Can't send packet.\n");
        }


    }

    return NULL;
}
//
// void get_neighbors(int **neighbors, int *n_neighbors, int distance_matrix[][n_routers]) {
//     int router_a, router_b, cost;
//
//     for (size_t i = 0; i < n_routers; i++) {
//         memset(distance_matrix[i], -1, sizeof(int)*n_routers);
//     }
//
//     FILE *links_settings = fopen("config/enlaces.config", "r");
//         for(size_t i = 0; fscanf(links_settings,"%d %d %d", &router_a, &router_b, &cost) != EOF; i++) {
//             if(router_a == self_router.id){
//                 *neighbors = (int *) realloc(*neighbors, sizeof(int) * ++(*n_neighbors));
//                 (*neighbors)[*n_neighbors-1] = router_b;
//                 distance_matrix[router_b-1][router_b-1] = cost;
//             }else if(router_b == self_router.id){
//                 *neighbors = (int *) realloc(*neighbors, sizeof(int) * ++(*n_neighbors));
//                 (*neighbors)[*n_neighbors-1] = router_a;
//                 distance_matrix[router_a-1][router_a-1] = cost;
//             }
//         }
//     fclose(links_settings);
// }

// int recalculate_did_change(int self_distance_vector[][n_routers], int distance_matrix[][n_routers]){
//     int min_cost;
//     int next_hop;
//     int changed = 0;
//     for (size_t j = 0; j < n_routers; j++) {
//         min_cost = -1;
//         next_hop = -1;
//         for (size_t i = 0; i < n_routers; i++) {
//             if(min_cost == -1 && distance_matrix[i][j] != -1) {
//                 min_cost = distance_matrix[i][j];
//                 next_hop = i;
//             }else if(distance_matrix[i][j] < min_cost && distance_matrix[i][j] != -1) {
//                 min_cost = distance_matrix[i][j];
//                 next_hop = i;
//             }
//         }
//         pthread_mutex_lock(&self_d_v_mutex);
//         if(min_cost != self_distance_vector[0][j]){
//
//             self_distance_vector[0][j] = min_cost;
//             self_distance_vector[1][j] = next_hop;
//             changed = 1;
//             pthread_mutex_unlock(&self_d_v_mutex);
//         }else{
//             pthread_mutex_unlock(&self_d_v_mutex);
//         }
//     }
//     return changed;
// }

router *get_neighbor_by_id(int id){
    for (size_t i = 0; i < n_neighbors; i++) {
        if(neighbors[i].id == id){
            return &neighbors[i];
        }
    }
}

int recalculate_self_d_v(){
    int changed = 0;
    for (size_t i = 0; i < n_neighbors; i++) {
        for (size_t j = 0; j < n_routers; j++) {

        }
    }
    return changed;
}

void send_to_neighbors(){
    packet to_send_d_v;
    to_send_d_v.type = D_V_TYPE;
    to_send_d_v.id_origin = self_router.id;
    memcpy(to_send_d_v.content, self_router.last_d_v, sizeof(int)*n_routers);
    for (size_t i = 0; i < n_neighbors; i++) {
        to_send_d_v.id_destination = neighbors[i].id;
        if(!sem_trywait(&d_v_buf_empty)) {
            pthread_mutex_lock(&d_v_buf_mutex);

            d_v_buf[d_v_buf_rear] = to_send_d_v;
            d_v_buf_rear = (d_v_buf_rear + 1) % D_V_BUF_LEN;

            pthread_mutex_unlock(&d_v_buf_mutex);

            sem_post(&d_v_buf_full);
        }
    }
}

static void *distance_vector(void *arg) {
    int d_v_buf_front = 0;
    int changed;
    packet d_v_packet;
    router *neighbor_router;
    struct timespec abs_tout;

    while (1) {
        if(sem_timedwait(&d_v_buf_full, &abs_tout)){

        }else{
            pthread_mutex_lock(&d_v_buf_mutex);

            d_v_packet = to_send_buf[d_v_buf_front];
            d_v_buf_front = (d_v_buf_front + 1) % D_V_BUF_LEN;

            pthread_mutex_unlock(&d_v_buf_mutex);
            sem_post(&d_v_buf_empty);

            neighbor_router = get_neighbor_by_id(d_v_packet.id_origin);
            clock_gettime(CLOCK_REALTIME, &neighbor_router->d_v_time);
            memcpy(neighbor_router->last_d_v, d_v_packet.content, N_MAX_ROUTERS*sizeof(int));

            changed = recalculate_self_d_v();
            if(changed){
                send_to_neighbors();
            }
        }
    }

    return NULL;
}

void new_neighbor(int neighbor_id, int neighbor_cost){
    neighbors = (router *) realloc(neighbors, sizeof(router) * (n_neighbors+1));
    neighbors[n_neighbors].id = neighbor_id;
    neighbors[n_neighbors].cost = neighbor_cost;
    neighbors[n_neighbors].available = 1;
    memset(neighbors[n_neighbors].last_d_v, -1, sizeof(int)*N_MAX_ROUTERS);
    n_neighbors++;
}

void create_neighbors(int self_id){
    int router_a_id, router_b_id, cost;

    FILE *links_settings = fopen("config/enlaces.config", "r");
        for(size_t i = 0; fscanf(links_settings,"%d %d %d", &router_a_id, &router_b_id, &cost) != EOF; i++) {
            if(router_a_id == self_id)
                new_neighbor(router_b_id, cost);
            else if(router_b_id == self_id)
                new_neighbor(router_a_id, cost);
        }
    fclose(links_settings);
}

int find_neighbor_by_id(int id){
    for (size_t i = 0; i < n_neighbors; i++) {
        if(neighbors[i].id == id){
            return i;
        }
    }
    return -1;
}


int get_routers_settings(int self_id) {
    create_neighbors(self_id);

    int flag = 1;
    char ip[15];
    int id;
    int port;
    int neighbor_index;
    FILE *routers_settings = fopen("config/roteador.config", "r");
        for(size_t i = 0, n_routers = 0; fscanf(routers_settings,"%d %d %s", &id, &port, ip) != EOF; i++, n_routers++) {
            if(id == self_id) {
                flag = 0;
                self_router.id = id;
                self_router.port = port;
                strcpy(self_router.ip, ip);
            }else if((neighbor_index = find_neighbor_by_id(id)) != -1) {
                neighbors[neighbor_index].port = port;
                strcpy(neighbors[neighbor_index].ip, ip);
            }
        }
    fclose(routers_settings);

    return flag;
}

// add the time of two timespec structures
struct timespec timespec_add(struct timespec t1, struct timespec t2) {
    long sec = t1.tv_sec + t2.tv_sec;
    long nsec = t1.tv_nsec + t2.tv_nsec;
    if (nsec >= 1000000000) {
        nsec -= 1000000000;
        sec += 1;
    }
    return (struct timespec) { .tv_sec = sec, .tv_nsec = nsec};
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
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(self_router.port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        printf(">>[ERROR] Can't bind socket.\n");
        return 1;
    }

    //
    self_distance_vector[0] = malloc(sizeof(int)*n_routers);
    memset(self_distance_vector[0], -1, sizeof(int)*n_routers);
    self_distance_vector[1] = malloc(sizeof(int)*n_routers);
    memset(self_distance_vector[1], -1, sizeof(int)*n_routers);
    //  initiate all semaphores
    sem_init(&to_send_buf_full, 0 , 0);
    sem_init(&to_send_buf_empty, 0, TO_SEND_BUF_LEN);

    sem_init(&d_v_buf_full, 0 , 0);
    sem_init(&d_v_buf_empty, 0, D_V_BUF_LEN);

    // create all needed threads
    pthread_t receiver_thread;
    pthread_t sender_thread;
    pthread_t distance_vector_thread;
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_create(&sender_thread, NULL, sender, NULL);
    pthread_create(&distance_vector_thread, NULL, distance_vector, NULL);

    // terminal interface to create new messages
    packet new_message;
    struct timespec abs_tout;
    struct timespec timeout;
    timeout.tv_sec = ACK_TOUT/1000;
    timeout.tv_nsec = (ACK_TOUT%1000)*1000000;

    while(1) {
        printf(">>To send a new message enter the ID of the destination followed by the message with up to %d caracteres\n>>Like this: '2 Hello router number 2!'\n", MESSAGE_LEN);

        scanf("%d ", &new_message.id_destination);
        fgets(new_message.content+sizeof(int), MESSAGE_LEN, stdin);
        new_message.type = 0;
        new_message.id_origin = self_router.id;
        *new_message.content = current_seq_num;

        if(!sem_trywait(&to_send_buf_empty)) {
            pthread_mutex_lock(&to_send_buf_mutex);

            to_send_buf[to_send_buf_rear] = new_message;
            to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

            pthread_mutex_unlock(&to_send_buf_mutex);

            sem_post(&to_send_buf_full);
        }else {
            printf(">>Package discarted because the buffer is full\n");
        }

        // wait for ack or timeout
        clock_gettime(CLOCK_REALTIME, &abs_tout);
        abs_tout = timespec_add(abs_tout, timeout);
        if(pthread_mutex_timedlock(&ack_mutex, &abs_tout)) {
            printf(">>Ack expired\n");
        }else{
            printf(">>Ack received\n");
        }

    }

    return 0;
}
