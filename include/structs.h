// #ifndef STRUCTS_H
// #define STRUCTS_H
#include <settings.h>

typedef struct router {
    int id;
    int port;
    char ip[15];
} router;

typedef struct routing_row {
    int id_destination;
    int id_next;
    int cost;
} routing_row;

// type 0 message packet
typedef struct message_packet {
    int type;
    int id_origin;
    int id_destination;
    int seq_num;
    char message[MESSAGE_LEN];
} message_packet;

// type 1 acknowledge packet
typedef struct ack_packet {
    int type;
    int id_origin;
    int id_destination;
    int seq_num;
} ack_packet;

// type 2 data packet
typedef struct data_packet {
    int type;
    int id_origin;
    int id_destination;
    int d_v[MAX_D_V_LEN];
} data_packet;
