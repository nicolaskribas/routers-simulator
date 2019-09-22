#include <writer.h>
#include <settings.h>
#include <structs.h>
#include <string.h> //memset()
#include <stdio.h>
#include <pthread.h> //phthread_t

void *writer(void *arg){
    package new_package;
    int try_count;

    while(TRUE){
        memset(&new_package, 0, sizeof(new_package));
        printf("Enter destination ID: ");
        scanf("%d", &new_package.id_destination);
        printf("Enter message: ");
        scanf("%s", new_package.message);
        new_package.id_origin = self_router.id;
        new_package.ack = FALSE;

        if(!sem_trywait(&to_send_buffer_empty)){
            pthread_mutex_lock(&to_send_buffer_mutex);

            to_send_buffer[to_send_buffer_rear] = new_package;
            to_send_buffer_rear = (to_send_buffer_rear + 1) % TO_SEND_BUFFER_LEN;

            pthread_mutex_unlock(&to_send_buffer_mutex);
            sem_post(&to_send_buffer_full);

            ack = TRUE;

            try_count = 1;



        }
    }

}
