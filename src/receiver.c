#include "receiver.h"
#include "settings.h"



void *receiver(void *arg){
    struct sockaddr_in server, client;
    int socklen = sizeof(server), recv_len;
    char buffer[sizeof(package)];

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
        memset(udp_message,'\0', UDP_MESSAGE_LEN;
        if ((recv_len = recvfrom(sock, upd_message, UDP_MESSAGE_LEN, 0, (struct sockaddr *) &client, &socklen)) == -1)
        {
            printf("[ERROR] receiving message\n");
            exit(1);
        }

        if(!sem_trywait(to_send_buffer_empty)){
            pthread_mutex_lock(to_send_buffer_mutex);

            strncpy(to_send_buffer[to_send_buffer_rear], udp_message, UDP_MESSAGE_LEN);
            to_send_buffer_rear = (to_send_buffer_rear + 1) % TO_SEND_BUFFER_LEN;

            pthread_mutex_unlock(to_send_buffer_mutex);

            sem_post(to_send_buffer_full);
        }else{
            printf("Package discarted\n");
        }


    }
}
