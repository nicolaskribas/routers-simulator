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
#include <unistd.h>

#define MSG_TYPE 0
#define ACK_TYPE 1
#define D_V_TYPE 2

// functions
router *get_neighbor_by_id(int id);
void send_to_neighbors(void);
struct timespec timespec_add(struct timespec t1, struct timespec t2);


// global variables

struct timespec resend_tout;
struct timespec receive_tout;


int next_hop[N_MAX_ROUTERS];

router self_router;     // ip/port of instanced router
int sock;               // socket used in the instanced router
int current_seq_num = 0;

router *neighbors = NULL;
int n_neighbors = 0;
int n_routers = 0;

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
        if((recv_len = recvfrom(sock, &buf, sizeof(packet), 0, (struct sockaddr *) &origin_addr, &addr_len)) == -1) {
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
                }else{
                    printf(">>Routing packet from %d to %d\n", buf.id_origin, buf.id_destination);
                }
                if(!sem_trywait(&to_send_buf_empty)) {
                    pthread_mutex_lock(&to_send_buf_mutex);

                    to_send_buf[to_send_buf_rear] = buf;
                    to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

                    pthread_mutex_unlock(&to_send_buf_mutex);

                    sem_post(&to_send_buf_full);
                }else {
                    printf(">>Package discarted because the sender buffer is full\n");
                }
            break;

            case ACK_TYPE: // for ack packets
                if(buf.id_destination == self_router.id) {
                    if(*(int *)(buf.content) == current_seq_num) {
                        pthread_mutex_unlock(&ack_mutex);
                    }
                }else {
                    printf(">>Routing ack packet from %d to %d\n", buf.id_origin, buf.id_destination);
                    if(!sem_trywait(&to_send_buf_empty)) {
                        pthread_mutex_lock(&to_send_buf_mutex);

                        to_send_buf[to_send_buf_rear] = buf;
                        to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

                        pthread_mutex_unlock(&to_send_buf_mutex);

                        sem_post(&to_send_buf_full);
                    }else {
                        printf(">>Package discarted because the sender buffer is full\n");
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
                    printf(">>Package discarted because the d-v buffer is full\n");
                }
            break;
        }
    }

    return NULL;
}

// int get_next(int id){
//     for (size_t i = 0; i < n_routers; i++) {
//         if(routing_table[i].id_destination == id){
//             return routing_table[i].id_next;
//         }
//     }
//     return -1;
// }

static void *sender(void *arg) {
    int to_send_buf_front = 0;
    router *next_router;
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

        if(to_send_packet.type == D_V_TYPE){
            next_router = get_neighbor_by_id(to_send_packet.id_destination);
        }else{
            next_router = get_neighbor_by_id(next_hop[to_send_packet.id_destination-1]);
        }
        if(next_router == NULL) continue;
        port = next_router->port;
        strcpy(ip, next_router->ip);

        if(!inet_aton(ip, &dest_addr.sin_addr)) {
            printf(">>[ERROR] inet_aton()\n");
        }
        dest_addr.sin_port = htons(port);

        if(sendto(sock, &to_send_packet, sizeof(packet), 0, (struct sockaddr *) &dest_addr, addr_len) == -1) {
            printf(">>[ERROR] Can't send packet.\n");
        }


    }

    return NULL;
}


router *get_neighbor_by_id(int id){
    for (size_t i = 0; i < n_neighbors; i++) {
        if(neighbors[i].id == id){
            return &neighbors[i];
        }
    }
    return NULL;
}

void print_d_v(){
    printf("\n\n>> Distance-vector changed:\n");
    printf("ID %d:", self_router.id);
    for (size_t j = 0; j < n_routers; j++) {
        printf(" %d", self_router.last_d_v[j]);
    }
    for (size_t i = 0; i < n_neighbors; i++) {
        printf("\nID %d:", neighbors[i].id);
        for (size_t j = 0; j < n_routers; j++) {
            printf(" %d", neighbors[i].last_d_v[j]);
        }
    }
    printf("\nPROXIMOS SALTOS: ");
    for (size_t i = 0; i < n_routers; i++) {
        printf(" %d", next_hop[i]);
    }
    printf("\n");
}

void recalculate_self_d_v(void){
    int changed = 0;
    int min;
    int min_index;
    for (size_t to = 0; to < n_routers; to++) {
        if(to == self_router.id-1) continue;
        min = -1;
        min_index = -1;
        for (size_t via = 0; via < n_neighbors; via++) {
            if(!neighbors[via].available || neighbors[via].last_d_v[to] == -1) continue;
            if(min == -1 || (neighbors[via].last_d_v[to] + neighbors[via].cost) < min){
                min = neighbors[via].last_d_v[to] + neighbors[via].cost;
                min_index = via;
            }
        }
        if(min > MAX_COST){
            self_router.last_d_v[to] = -1;
            next_hop[to] = -1;
            changed = 1;
        }else if(min != self_router.last_d_v[to]){
            self_router.last_d_v[to] = min;
            next_hop[to] = neighbors[min_index].id;
            changed = 1;
        }else if(min == self_router.last_d_v[to]){
            next_hop[to] = neighbors[min_index].id;
        }

    }
    if(changed){
        print_d_v();
        send_to_neighbors();
    }
}

void send_to_neighbors(void){
    packet to_send_d_v;
    to_send_d_v.type = D_V_TYPE;
    to_send_d_v.id_origin = self_router.id;
    memcpy(to_send_d_v.content, self_router.last_d_v, sizeof(int)*n_routers);
    clock_gettime(CLOCK_REALTIME, &self_router.d_v_time);
    self_router.d_v_time = timespec_add(self_router.d_v_time, resend_tout);
    for (size_t i = 0; i < n_neighbors; i++) {
        to_send_d_v.id_destination = neighbors[i].id;
        if(!sem_trywait(&to_send_buf_empty)) {
            pthread_mutex_lock(&to_send_buf_mutex);

            to_send_buf[to_send_buf_rear] = to_send_d_v;
            to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

            pthread_mutex_unlock(&to_send_buf_mutex);

            sem_post(&to_send_buf_full);
        }else{
            printf(">>Package discarted because the sender buffer is full\n");
        }
    }
}

int is_smaller(struct timespec left, struct timespec right){
    if (left.tv_sec == right.tv_sec)
        return (left.tv_nsec < right.tv_nsec);
    else
        return (left.tv_sec < right.tv_sec);
}

struct timespec check_tout(void) {
    struct timespec current;
    struct timespec next_tout;
    clock_gettime(CLOCK_REALTIME, &current);
    if(is_smaller(self_router.d_v_time, current)){
        send_to_neighbors();
    }
    next_tout = self_router.d_v_time;
    for (size_t i = 0; i < n_neighbors; i++) {
        if(neighbors[i].available){
            if(is_smaller(neighbors[i].d_v_time, current)){
                neighbors[i].available = 0;
                recalculate_self_d_v();
            }else if(is_smaller(neighbors[i].d_v_time, next_tout)){
                next_tout = neighbors[i].d_v_time;
            }
        }
    }

    return next_tout;
}

static void *distance_vector(void *arg) {
    resend_tout.tv_sec = RESEND_TOUT/1000;
    resend_tout.tv_nsec = (RESEND_TOUT%1000)*1000000;
    receive_tout.tv_sec = (RESEND_TOUT*3)/1000;
    receive_tout.tv_nsec = ((RESEND_TOUT*3)%1000)*1000000;

    int d_v_buf_front = 0;
    packet d_v_packet;
    router *neighbor_router;
    struct timespec next_tout;
    print_d_v();
    send_to_neighbors();
    next_tout = check_tout();
    while (1) {
        if(sem_timedwait(&d_v_buf_full, &next_tout)){
            next_tout = check_tout();
        }else{
            pthread_mutex_lock(&d_v_buf_mutex);

            d_v_packet = d_v_buf[d_v_buf_front];
            d_v_buf_front = (d_v_buf_front + 1) % D_V_BUF_LEN;

            pthread_mutex_unlock(&d_v_buf_mutex);
            sem_post(&d_v_buf_empty);

            neighbor_router = get_neighbor_by_id(d_v_packet.id_origin);
            neighbor_router->available = 1;
            clock_gettime(CLOCK_REALTIME, &neighbor_router->d_v_time);
            neighbor_router->d_v_time = timespec_add(neighbor_router->d_v_time, receive_tout);
            memcpy(neighbor_router->last_d_v, d_v_packet.content, N_MAX_ROUTERS*sizeof(int));

            recalculate_self_d_v();
            next_tout = check_tout();
        }
    }

    return NULL;
}

void new_neighbor(int neighbor_id, int neighbor_cost){
    neighbors = (router *) realloc(neighbors, sizeof(router) * (n_neighbors+1));
    neighbors[n_neighbors].id = neighbor_id;
    neighbors[n_neighbors].cost = neighbor_cost;
    neighbors[n_neighbors].available = 0;
    memset(neighbors[n_neighbors].last_d_v, -1, sizeof(int)*N_MAX_ROUTERS);

    self_router.last_d_v[neighbor_id-1] = neighbor_cost;

    next_hop[neighbor_id-1] = neighbor_id;
    // routing_table[neighbor_id-1].id_next = neighbor_id;
    // routing_table[neighbor_id-1].cost = neighbor_cost;

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
    memset(self_router.last_d_v, -1, sizeof(int)*N_MAX_ROUTERS);
    self_router.last_d_v[self_id-1] = 0;

    create_neighbors(self_id);

    int flag = 1;
    char ip[15];
    int id;
    int port;
    int neighbor_index;
    FILE *routers_settings = fopen("config/roteador.config", "r");
        for(size_t i = 0; fscanf(routers_settings,"%d %d %s", &id, &port, ip) != EOF; i++, n_routers++) {
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

static void *writer_thread(void *args) {
    // terminal interface to create new messages
    packet new_message;
    struct timespec abs_tout;
    struct timespec timeout;
    timeout.tv_sec = ACK_TOUT/1000;
    timeout.tv_nsec = (ACK_TOUT%1000)*1000000;
    printf(">>To send a new message enter the ID of the destination followed by the message with up to %d caracteres\n>>Like this: '2 Hello router number 2!'\n", MESSAGE_LEN);
    while(1) {
        scanf("%d ", &new_message.id_destination);
        fgets(new_message.content+sizeof(int), MESSAGE_LEN, stdin);
        new_message.type = MSG_TYPE;
        new_message.id_origin = self_router.id;
        *new_message.content = ++current_seq_num;

        if(!sem_trywait(&to_send_buf_empty)) {
            pthread_mutex_lock(&to_send_buf_mutex);

            to_send_buf[to_send_buf_rear] = new_message;
            to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

            pthread_mutex_unlock(&to_send_buf_mutex);

            sem_post(&to_send_buf_full);
        }else {
            printf(">>Package discarted because the sender buffer is full\n");
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
    return NULL;
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
    // for (size_t i = 0; i < n_routers; i++) {
    //     routing_table[i].id_destination = i+1;
    // }
    memset(&next_hop, -1, sizeof(int) * N_MAX_ROUTERS);
    // create a socket
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
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

    //  initiate all semaphores
    sem_init(&to_send_buf_full, 0 , 0);
    sem_init(&to_send_buf_empty, 0, TO_SEND_BUF_LEN);

    sem_init(&d_v_buf_full, 0 , 0);
    sem_init(&d_v_buf_empty, 0, D_V_BUF_LEN);
    pthread_mutex_lock(&ack_mutex);
    // create all needed threads
    pthread_t receiver_thread;
    pthread_t sender_thread;
    pthread_t distance_vector_thread;
    pthread_t writer;
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_create(&sender_thread, NULL, sender, NULL);
    pthread_create(&distance_vector_thread, NULL, distance_vector, NULL);
    pthread_create(&writer, NULL, writer_thread, NULL);

    pthread_join(writer, NULL);
    return 0;
}
