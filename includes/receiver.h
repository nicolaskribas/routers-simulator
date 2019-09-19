#ifndef RECEIVER_H
#define RECEIVER_H

typedef struct receiver_args{
    package *to_send_buffer;
    pthread_mutex_t *to_send_buffer_mutex;
    sem_t *to_send_buffer_full;
    sem_t *to_send_buffer_empty;
    int *next_empty;
    router self_router;
}receiver_args;

void *receiver(void *arg);
#endif
