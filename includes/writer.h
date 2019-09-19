#ifndef WRITER_H
#define WRITER_H

typedef struct writer_args{
    package *to_send_buffer;
    pthread_mutex_t *to_send_buffer_mutex;
    sem_t *to_send_buffer_full;
    sem_t *to_send_buffer_empty;
    int *next_empty;
    router self_router;
}writer_args;

void *writer(void *arg);
#endif
