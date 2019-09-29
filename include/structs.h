// #ifndef STRUCTS_H
// #define STRUCTS_H
#include <settings.h>
#include <semaphore.h>

#define TRUE 1
#define FALSE 0

typedef struct router{
    int id;
    int port;
    char ip[15];
}router;

typedef struct routing_row{
    int id_destination;
    int id_next;
    int cost;
}routing_row;

typedef struct package{
    int id_origin;
    int id_destination;
    int ack;
    int seq_num;
    char message[MESSAGE_LEN];
}package;

// extern int to_send_buffer_rear;                            // A traseira da fila circular
// extern sem_t to_send_buffer_full, to_send_buffer_empty;    //Semafaros produtor-consumidor
// extern pthread_mutex_t to_send_buffer_mutex;               //Mutex região critica da fila
// extern package to_send_buffer[TO_SEND_BUFFER_LEN];         //Fila circular
// extern char ack;
// extern int mysocket;
// extern router self_router;
// extern struct sockaddr_in si_me;
extern routing_row *routing_table;
// extern router *routers;
// extern int n_routers;

// #endif
