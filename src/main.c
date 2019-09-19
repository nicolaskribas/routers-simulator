#include "main.h"
#include "settings.h"
#include "receiver.h"
#include "sender.h"
#include "writer.h"


int get_routers_settings(int *n_routers, router **routers, int self_id, router *self_router)
{
    int  i, flag = TRUE;
    router aux;
    FILE *routers_settings = fopen("../config/roteador.config", "r");
        for(i = 0, *n_routers = 0; fscanf(routers_settings,"%d %d %s", &aux.id, &aux.port, aux.ip) != EOF; i++)
        {
            *routers = realloc (*routers, sizeof(router) * ++(*n_routers));

            (*routers)[i].id = aux.id;
            (*routers)[i].port = aux.port;
            sprintf((*routers)[i].ip, "%s", aux.ip);

            if(aux.id == self_id){
                flag = FALSE;
                *self_router = aux;
            }
        }
    fclose(routers_settings);

    if(flag){
        printf("[ERROR] Router with ID %d not configurated.\n", self_id);
        exit(1);
    }
    return(0);

    // int a, b, cost;
    // FILE *enlaces = fopen("../config/enlaces.config", "r");
    //     for(i = 0, n_edges = 0; fscanf(elaces,"%d %d %d", %a, %b, %cost) != EOF; i++)
    //     {
    //
    //
    //     }
    // fclose(enlaces);
}

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        exit(1);
    }
    int self_id = atoi(argv[1]);

    int n_routers;
    router *self_router, *routers = NULL;
    self_router = malloc(sizeof(router));
    get_routers_settings(&n_routers, &routers, self_id, self_router);

    // inicia mutex e semafaros
    pthread_mutex_t to_send_buffer_mutex;
    sem_t to_send_buffer_full, to_send_buffer_empty,;
    sem_init(&to_send_buffer_full, 0 , 0);
    sem_init(&to_send_buffer_empty, 0, TO_SEND_BUFFER_LEN);

    // inicia variáveis compartilhadas entre threads
    int next_empty = 0;
    package to_send_buffer[TO_SEND_BUFFER_LEN];
    char ack = "F";

    // controi argumentos das threads
    receiver_args Receiver_args;
    Receiver_args.to_send_buffer = to_send_buffer;
    Receiver_args.to_send_buffer_mutex = &to_send_buffer_mutex;
    Receiver_args.to_send_buffer_full = &to_send_buffer_full;
    Receiver_args.to_send_buffer_empty = &to_send_buffer_empty;
    Receiver_args.self_router = *self_router;
    Receiver_args.next_empty = &next_empty;

    sender_args Sender_args;
    Receiver_args.to_send_buffer = to_send_buffer;
    Sender_args.to_send_buffer_mutex = &to_send_buffer_mutex;
    Sender_args.to_send_buffer_full = &to_send_buffer_full;
    Sender_args.to_send_buffer_empty = &to_send_buffer_empty;
    Sender_args.self_router = *self_router;

    writer_args Writer_args = (writer_args) Receiver_args;









    // inicia threads
    pthread_t Receiver, Sender, Writer;

    // pthread_create(&Receiver, NULL, receiver, &Receiver_args);
    // pthread_create(&Sender, NULL, sender, &Sender_args);
    // pthread_create(&Writer, NULL, writer, &Writer_args);
    //
    // pthread_join(Receiver, NULL);
    // pthread_join(Sender, NULL);
    // pthread_join(Writer, NULL);

    return(0);
}
