#include "sender.h"
#include "settings.h"

void *sender(void *arg){
    sender_args *Sender_args = (sender_args *) arg;

    package *to_send_buffer = Sender_args.to_send_buffer;
    pthread_mutex_t *to_send_buffer_mutex = Sender_args.to_send_buffer_mutex;
    sem_t *to_send_buffer_full = Sender_args.to_send_buffer_full;
    sem_t *to_send_buffer_empty = Sender_args.to_send_buffer_empty;
    int *next_empty = Sender_args.next_empty;
    router self_router = Sender_args->self_router;

    int next_full = 0;
    while(1){
        sem_wait(to_send_buffer_full);
        pthread_mutex_lock(to_send_buffer_mutex);
        // pega mensagem
        pack = to_send_buffer[next_full];
        next_full++;
        pthread_mutex_unlock(to_send_buffer_mutex);
        sem_post(to_send_buffer_empty);
        // envia mensagem
    }
}
