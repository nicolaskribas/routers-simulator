#include <stdio.h>      // printf()
#include <stdlib.h>     // atoi()
#include <pthread.h>    // threads
#include <structs.h>    // router
#include <settings.h>   // message len
#include <sys/socket.h> // socket()
#include <arpa/inet.h>  // IPPROTO_UDP
#include <semaphore.h>  //
#include <time.h>       // timespec

// global variables
int n_routers;
router *routers = NULL; // ip/port of all the routers
router self_router;     // ip/port of instanced router
int sock;               // socket used in the instanced router
int current_seq_num = 0;

pthread_mutex_t ack_mutex;

pthread_mutex_t to_send_buf_mutex;
sem_t to_send_buf_full;
sem_t to_send_buf_empty;
data_packet to_send_buf[TO_SEND_BUF_LEN];
int to_send_buf_rear = 0;

pthread_mutex_t d_v_buf_mutex;
sem_t d_v_buf_full;
sem_t d_v_buf_empty;
data_packet d_v_buf[D_V_BUF_LEN];
int d_v_buf_rear = 0;

static void *receiver(void *arg) {
    int recv_len;
    char buf[MAX_LEN];
    socklen_t addr_len;
    sockaddr_in origin_addr;
    int old_origin;
    while(1) {
        if((recv_len = recvfrom(sock, buf, MAX_LEN, 0, (sockaddr *) &origin_addr, &addr_len))) {
            printf(">>[ERROR] Can't receive packet.\n");
        }

        switch(*(int *) buf) {
            case 0: // for message packets
                if(((message_packet *)buf)->id_destination == self_router.id) {
                    printf(">>New message from router %d: %s\n", ((message_packet *)buf)->id_origin, ((message_packet *)buf)->message);
                    ((ack_packet *)buf)->type = 1;
                    old_origin = ((ack_packet *)buf)->id_origin;
                    ((ack_packet *)buf)->id_origin = ((message_packet *)buf)->id_destination;
                    ((ack_packet *)buf)->id_destination = old_origin;
                }
                if(!sem_trywait(&to_send_buf_empty)) {
                    pthread_mutex_lock(&to_send_buf_mutex);

                    to_send_buf[to_send_buf_rear] = *(data_packet *)buf;
                    to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

                    pthread_mutex_unlock(&to_send_buf_mutex);

                    sem_post(&to_send_buf_full);
                }else {
                    printf(">>Package discarted because the buffer is full\n");
                }
            break;

            case 1: // for ack packets
                if(((ack_packet *)buf)->id_destination == self_router.id) {
                    if(((ack_packet *)buf)->seq_num == current_seq_num) {
                        pthread_mutex_unlock(&ack_mutex);
                    }
                }else {
                    if(!sem_trywait(&to_send_buf_empty)) {
                        pthread_mutex_lock(&to_send_buf_mutex);

                        to_send_buf[to_send_buf_rear] = *(data_packet *)buf;
                        to_send_buf_rear = (to_send_buf_rear + 1) % TO_SEND_BUF_LEN;

                        pthread_mutex_unlock(&to_send_buf_mutex);

                        sem_post(&to_send_buf_full);
                    }else {
                        printf(">>Package discarted because the buffer is full\n");
                    }
                }
            break;

            case 2: // for distance-vector packets
                if(!sem_trywait(&d_v_buf_empty)) {
                    pthread_mutex_lock(&d_v_buf_mutex);

                    d_v_buf[d_v_buf_rear] = *(data_packet *)buf;
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

    data_packet to_send_packet;
    int size;

    sockaddr_in dest_addr;
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

        switch(to_send_packet.type) {
            case 0:
                size = sizeof(message_packet);
                break;
            case 1:
                size = sizeof(ack_packet);
                break;
            case 2:
                size = sizeof(data_packet);
                break;
        }

        // for (int i = 0; i < (n_routers-1); i++) {
        //     if(to_send_packet.id_destination == routing_table[i].id_destination){
        //         for (int j = 0; j < n_routers; j++) {
        //             if(routing_table[i].id_next == routers[j].id){
        //                 ip = routers[j].ip;
        //                 port = routers[j].port;
        //             }
        //         }
        //     }
        // }

        if(inet_aton(ip, &dest_addr.sin_addr)) {
            printf(">>[ERROR] inet_aton()\n");
        }
        dest_addr.sin_port = htons(port);

        if(sendto(sock, &to_send_packet, size, 0, (sockaddr *) &dest_addr, addr_len)) {
            printf(">>[ERROR] Can't send packet.\n");
        }


    }

    return NULL;
}

static void *distance_vector(void *arg) {
    while (1) {

    }

    return NULL;
}

int get_routers_settings(int self_id) {
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

    if(flag) {
        return 1;
    }

    return 0;
}

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
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(self_router.port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (sockaddr *) &addr, sizeof(addr))) {
        printf(">>[ERROR] Can't bind socket.\n");
        return 1;
    }

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
    message_packet new_message;
    struct timespec abs_tout;
    struct timespec timeout;
    timeout.tv_sec = ACK_TOUT/1000;
    timeout.tv_nsec = (ACK_TOUT%1000)*1000000;

    while(1) {
        printf(">>To send a new message enter the ID of the destination followed by the message with up to %d caracteres\n>>Like this: '2 Hello router number 2!'\n", MESSAGE_LEN);

        scanf("%d ", &new_message.id_destination);
        fgets(new_message.message, MESSAGE_LEN, stdin);
        new_message.type = 0;
        new_message.id_origin = self_router.id;
        new_message.seq_num = current_seq_num;

        if(!sem_trywait(&to_send_buf_empty)) {
            pthread_mutex_lock(&to_send_buf_mutex);

            to_send_buf[to_send_buf_rear] = *(data_packet *)(&new_message);
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
