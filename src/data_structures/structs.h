#ifndef STRUCTS
#define STRUCTS

#include <stdio.h>  //printf, scanf
#include <stdlib.h> //realloc
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<string.h> //memset

#define BUFFERLEN 100
#define TRUE 1
#define FALSE 0

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct router{
    int id;
    int port;
    char ip[15];
}router;

typedef struct package{
    int id_origin;
    int id_destination;
    char ack;
    char message[BUFFERLEN];
}package;

#endif
