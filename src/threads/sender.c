#include "../data_structures/structs.h"
void next(void){
    if(start_buffer == BUFFERLEN-1){
        start_buffer = 0;
    }else{
        start_buffer++;
    }
}
void *sender(void *arg){
    package pack;
    while(1){
        sem_wait(&full);
        pthread_mutex_lock(&to_send_buffer_mutex);
        // pega mensagem
        pack = to_send_buffer[start_buffer];
        next();
        pthread_mutex_unlock(&to_send_buffer_mutex);
        sem_post(&empty);
        // envia mensagem
    }
}
