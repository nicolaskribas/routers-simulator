#include "../data_structures/structs.h"
void *receiver(void *arg){
    router *self_router = (router *) arg;
    struct sockaddr_in server, client;
    int sock, socklen = sizeof(server), recv_len;
    char buffer[BUFFERLEN];

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
        memset(buffer,'\0', BUFFERLEN);
        if ((recv_len = recvfrom(sock, buffer, BUFFERLEN, 0, (struct sockaddr *) &client, &socklen)) == -1)
        {
            printf("[ERROR] receiving message\n");
            exit(1);
        }
        pthread_mutex_lock(&to_send_buffer_mutex);
        pthread_mutex_unlock(&to_send_buffer_mutex);

    }
}
