#include "receiver.h"
#include "settings.h"

package unkpack(char message[]){
    package pack;
    return pack;
}

void *receiver(void *arg){
    receiver_args *Receiver_args = (receiver_args *) arg;
    package *to_send_buffer = Receiver_args.to_send_buffer;
    pthread_mutex_t *to_send_buffer_mutex = Receiver_args.to_send_buffer_mutex;
    sem_t *to_send_buffer_full = Receiver_args.to_send_buffer_full;
    sem_t *to_send_buffer_empty = Receiver_args.to_send_buffer_empty;
    int *next_empty = Receiver_args.next_empty;
    router self_router = Receiver_args->self_router;

    struct sockaddr_in server, client;
    int sock, socklen = sizeof(server), recv_len;
    char buffer[sizeof(package)];

    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        printf("[ERROR] receiver socket creation\n");
        exit(1);
    }

    memset((char *) &server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(self_router->port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) == -1){
        printf("[ERROR] receiver socket binding\n");
        exit(1);
    }

    while(1){
        fflush(stdout);
        memset(buffer,'\0', sizeof(package));
        if ((recv_len = recvfrom(sock, buffer, sizeof(package), 0, (struct sockaddr *) &client, &socklen)) == -1)
        {
            printf("[ERROR] receiving message\n");
            exit(1);
        }

        if(!sem_trywait(to_send_buffer_empty)){
            pthread_mutex_lock(to_send_buffer_mutex);
            to_send_buffer[*next_empty] = pack(buffer);
            // adicionar o next_empty
            pthread_mutex_unlock(to_send_buffer_mutex);

            sem_post(to_send_buffer_full);
        }else{
            printf("Package discarted\n");
        }


    }
}
