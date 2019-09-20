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

int to_send_buffer_rear;                            // A traseira da fila circular
sem_t to_send_buffer_full, to_send_buffer_empty;    //Semafaros produtor-consumidor
pthread_mutex_t to_send_buffer_mutex;               //Mutex região critica da fila
package to_send_buffer[TO_SEND_BUFFER_LEN];         //Fila circular
char ack;
int socket;
int main(int argc, char *argv[]){

    if(argc != 2){
        printf("[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        exit(1);
    }
    int self_id = atoi(argv[1]);

    int n_routers;
    router *self_router, *routers = NULL;
    get_routers_settings(&n_routers, &routers, self_id, self_router);

    if ( (socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        puts("socket");
    }
    ack = "F";
    to_send_buffer_rear = 0;
    // inicia semafaros
    sem_init(&to_send_buffer_full, 0 , 0);
    sem_init(&to_send_buffer_empty, 0, TO_SEND_BUFFER_LEN);

    // inicia threads
    pthread_t Receiver, Sender, Writer;

    pthread_create(&Receiver, NULL, receiver, NULL);
    pthread_create(&Sender, NULL, sender, NULL);
    pthread_create(&Writer, NULL, writer, NULL);

    pthread_join(Receiver, NULL);
    pthread_join(Sender, NULL);
    pthread_join(Writer, NULL);

    return(0);
}
