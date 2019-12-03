// #ifndef STRUCTS_H
// #define STRUCTS_H
#include <settings.h>
#include <time.h>

typedef struct router {
    int id;
    int port;
    char ip[15];
    int cost;
    int available;
    struct timespec d_v_time;
    int last_d_v[N_MAX_ROUTERS];
} router;

typedef struct routing_row {
    int id_destination;
    int id_next;
    int cost;
} routing_row;

typedef struct packet {
    int type;
    int id_origin;
    int id_destination;
    char content[CONTENT_MAX_LEN];
} packet;
