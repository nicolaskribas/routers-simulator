#include "sender.h"
#include <structs.h>
#include <settings.h>
#include <netinet/in.h> //sockaddr_in
#include <string.h> //memset()
#include <pthread.h> //phthread_t
#include <stdio.h>
#include <arpa/inet.h>

void send_package(package to_send_package){
    struct sockaddr_in si_other;
    memset(&si_other, 0, sizeof(si_other));
    int slen;
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(8888); // AQUI VAI A PORTA DE ACORDO COM O DIKSTRA

    if (inet_aton("127.0.0.1", &si_other.sin_addr) == 0) // AQUI VAI O IP DE ACORDO COM O DIKSTRA
    {
        printf("inet_aton() failed\n");
        printf("Deu probllema\n");
    }

    if (sendto(mysocket, &to_send_package, sizeof(to_send_package) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        printf("Deu probrema\n");
    }
}

void *sender(void *arg){
    int to_send_buffer_front = 0;
    package to_send_package;

    while(1){
        sem_wait(&to_send_buffer_full);
        pthread_mutex_lock(&to_send_buffer_mutex);

        to_send_package = to_send_buffer[to_send_buffer_front];
        to_send_buffer_front = (to_send_buffer_front + 1) % TO_SEND_BUFFER_LEN;

        pthread_mutex_unlock(&to_send_buffer_mutex);
        sem_post(&to_send_buffer_empty);

        send_package(to_send_package);
    }
}
