#include "receiver.h"
#include "settings.h"

void *receiver(void *arg){
    struct sockaddr_in si_other;
    int socketlen = sizeof(si_other), recv_len;
    package received_package;

    if(bind(socket, (struct sockaddr*)&si_me, sizeof(si_me)) == -1){
        printf("[ERROR] receiver socket binding\n");
        exit(1);
    }

    while(1){
        fflush(stdout);
        memset(received_package, 0, sizeof(received_package));
        if ((recv_len = recvfrom(socket, &received_package, sizeof(received_package), 0, (struct sockaddr *) &si_other, &socketlen)) == -1)
        {
            printf("[ERROR] receiving message\n");
            exit(1);
        }
        printf("Pacote passando %d -> %d\n", received_package.id_origin, received_package.id_destination);
        if(received_package.id_destination != self_router.id){}
            if(!sem_trywait(to_send_buffer_empty)){
                pthread_mutex_lock(to_send_buffer_mutex);

                to_send_buffer[to_send_buffer_rear] = received_package;
                to_send_buffer_rear = (to_send_buffer_rear + 1) % TO_SEND_BUFFER_LEN;

                pthread_mutex_unlock(to_send_buffer_mutex);

                sem_post(to_send_buffer_full);
            }else{
                printf("Package discarted\n");
            }
        }


    }
}
