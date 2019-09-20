#include "sender.h"
#include "settings.h"

void *sender(void *arg){
    int to_send_buffer_front = 0;
    package udp_message;
    struct sockaddr_in si_other;
    char message[MESSAGE_LEN];
    while(1){
        sem_wait(to_send_buffer_full);
        pthread_mutex_lock(to_send_buffer_mutex);

        strncpy(udp_message, (package) to_send_buffer[to_send_buffer_front], sizeof(package));
        to_send_buffer_front = (to_send_buffer_front + 1) % TO_SEND_BUFFER_LEN;
        pthread_mutex_unlock(to_send_buffer_mutex);
        sem_post(to_send_buffer_empty);

    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    if (sendto(socket, (char *) udp_message, sizeof(package) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        die("sendto()");
    }
}
