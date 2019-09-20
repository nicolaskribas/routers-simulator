#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct router{
    int id;
    int port;
    char ip[15];
}router;

typedef struct package{
    int id_origin;
    int id_destination;
    char ack;
    char message[MESSAGE_LEN];
}package;


#endif
