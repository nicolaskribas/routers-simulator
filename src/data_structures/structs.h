#ifndef STRUCTS
#define STRUCTS

#include <stdio.h>  //printf, scanf
#include <stdlib.h> //realloc

#define BUFFERLEN 100

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
