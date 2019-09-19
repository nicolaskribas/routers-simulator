#ifndef SENDER_H
#define SENDER_H

typedef struct sender_args{
    package *to_send_buffer;
    pthread_mutex_t *to_send_buffer_mutex;
    sem_t *to_send_buffer_full;
    sem_t *to_send_buffer_empty;
    router self_router;
}sender_args;

void *writer(void *arg);
#endif
