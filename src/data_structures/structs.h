#ifndef STRUCTS
#define STRUCTS

#include <stdio.h>  //printf, scanf
#include <stdlib.h> //realloc
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h> //memset
#include <semaphore.h> //sem_t

#define BUFFERLEN 2
#define MESSAGELEN 100
#define TRUE 1
#define FALSE 0

pthread_mutex_t to_send_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t full;
sem_t empty;
typedef struct router{
    int id;
    int port;
    char ip[15];
}router;

typedef struct package{
    int id_origin;
    int id_destination;
    char ack;
    char message[MESSAGELEN];
}package;

package to_send_buffer[BUFFERLEN];
int start_buffer;
int end_buffer;
#endif
