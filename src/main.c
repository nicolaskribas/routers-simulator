//COLOCAR \n na hora de printar a mensagem
//tirar o espaço do começo da string
//tirar \n do final da mensagem
#include <settings.h>
#include <structs.h>
#include <semaphore.h> //sem_t
#include <stdio.h>  //scanf()
#include <stdlib.h> //atoi()
#include <string.h> //memset()
#include <sys/socket.h> //socket()
#include <arpa/inet.h> //IPPROTO_UDP
#include <pthread.h> //phthread_t
#include <routing_table.h>
#include <time.h>
#include <stdio_ext.h>

int to_send_buffer_rear, ack, seq_num = 0, mysocket, n_routers = 0;
sem_t to_send_buffer_full, to_send_buffer_empty;    //Semafaros produtor-consumidor
pthread_mutex_t to_send_buffer_mutex, ack_mutex;    //Mutex região critica da fila
package to_send_buffer[TO_SEND_BUFFER_LEN];         //Fila circular
struct sockaddr_in si_me;
routing_row *routing_table = NULL;
router *routers = NULL;
router self_router;
int swap;
void *receiver(void *arg){
    struct sockaddr_in si_other;
    int socketlen = sizeof(si_other), recv_len;
    package received_package;

    if(bind(mysocket, (struct sockaddr*)&si_me, sizeof(si_me)) == -1){
        printf(">>[ERROR] bind()\n");
        exit(1);
    }

    while(1){
        fflush(stdout);
        memset(&received_package, 0, sizeof(received_package));
        if ((recv_len = recvfrom(mysocket, &received_package, sizeof(received_package), 0, (struct sockaddr *) &si_other, &socketlen)) == -1)
        {
            printf(">>[ERROR] recvfrom()\n");
            exit(1);
        }
        if(received_package.id_destination != self_router.id){
            printf(">>Routing package %d from %d to %d\n", received_package.seq_num, received_package.id_origin, received_package.id_destination);
            if(!sem_trywait(&to_send_buffer_empty)){
                pthread_mutex_lock(&to_send_buffer_mutex);

                to_send_buffer[to_send_buffer_rear] = received_package;
                to_send_buffer_rear = (to_send_buffer_rear + 1) % TO_SEND_BUFFER_LEN;

                pthread_mutex_unlock(&to_send_buffer_mutex);

                sem_post(&to_send_buffer_full);
            }else{
                printf(">>Package discarted\n");
            }
        }else if(received_package.ack == TRUE){
            pthread_mutex_lock(&ack_mutex);
            if(ack == TRUE && seq_num == received_package.seq_num){
                ack = FALSE;
            }
            pthread_mutex_unlock(&ack_mutex);
        }else{
            printf(">>New message from %d: %s", received_package.id_origin, received_package.message);
            received_package.ack = TRUE;
            swap = received_package.id_destination;
            received_package.id_destination = received_package.id_origin;
            received_package.id_origin = swap;
            if(!sem_trywait(&to_send_buffer_empty)){
                pthread_mutex_lock(&to_send_buffer_mutex);

                to_send_buffer[to_send_buffer_rear] = received_package;
                to_send_buffer_rear = (to_send_buffer_rear + 1) % TO_SEND_BUFFER_LEN;

                pthread_mutex_unlock(&to_send_buffer_mutex);

                sem_post(&to_send_buffer_full);
            }else{
                printf(">>Package discarted\n");
            }
        }
    }
}

void send_package(package to_send_package){
    int i, j;
    for(i = 0; i < n_routers-1; i++){
        if(to_send_package.id_destination == routing_table[i].id_destination){
            break;
        }
    }
    for(j = 0; j < n_routers; j++){
        if(routing_table[i].id_next == routers[j].id){
            break;
        }
    }

    struct sockaddr_in si_other;

    memset(&si_other, 0, sizeof(si_other));
    int slen = sizeof(si_other);
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(routers[j].port); // AQUI VAI A PORTA DE ACORDO COM O DIKSTRA

    if (inet_aton(routers[j].ip, &si_other.sin_addr) == 0) // AQUI VAI O IP DE ACORDO COM O DIKSTRA
    {
        printf(">>[ERROR] inet_aton()\n");
        exit(1);
    }

    if (sendto(mysocket, &to_send_package, sizeof(to_send_package) , 0 , (struct sockaddr *) &si_other, slen)==-1){
        printf(">>[ERROR] sento()\n");
        exit(1);
    }
}

void *sender(void *arg){
    int to_send_buffer_front = 0;
    package to_send_package;

    while(1){
        sem_wait(&to_send_buffer_full);
        pthread_mutex_lock(&to_send_buffer_mutex);

        to_send_package = to_send_buffer[to_send_buffer_front];
        to_send_buffer_front = (to_send_buffer_front + 1) % TO_SEND_BUFFER_LEN;

        pthread_mutex_unlock(&to_send_buffer_mutex);
        sem_post(&to_send_buffer_empty);

        send_package(to_send_package);
    }
}

void *writer(void *arg){
    package new_package;
    clock_t start, current;
    printf(">>To send a new message enter the ID of the destination followed by the message with up to %d caracteres\n>>Like this: '2 Hello router number 2!'\n", MESSAGE_LEN);
    while(TRUE){
        memset(&new_package, 0, sizeof(new_package));
        __fpurge(stdin);
        scanf("%d", &new_package.id_destination);
        if(new_package.id_destination == self_router.id || new_package.id_destination > n_routers || new_package.id_destination<=0){
            printf(">>Invalid ID\n");
            continue;
        }
        fgets(new_package.message, MESSAGE_LEN, stdin);
        new_package.id_origin = self_router.id;
        new_package.ack = FALSE;
        new_package.seq_num = seq_num;
        if(!sem_trywait(&to_send_buffer_empty)){
            pthread_mutex_lock(&to_send_buffer_mutex);

            pthread_mutex_lock(&ack_mutex);
            ack = TRUE;
            pthread_mutex_unlock(&ack_mutex);

            to_send_buffer[to_send_buffer_rear] = new_package;
            to_send_buffer_rear = (to_send_buffer_rear + 1) % TO_SEND_BUFFER_LEN;
            pthread_mutex_unlock(&to_send_buffer_mutex);
            sem_post(&to_send_buffer_full);


            start = clock();
            while( (clock()/CLOCKS_PER_SEC) < ((start/CLOCKS_PER_SEC) + TIMEOUT) ){
                pthread_mutex_lock(&ack_mutex);
                if(ack == FALSE){
                    pthread_mutex_unlock(&ack_mutex);
                    printf(">>Ack for package %d received\n", seq_num);
                    goto next;
                }
                pthread_mutex_unlock(&ack_mutex);
            }
            printf(">>Timeout reached, ack was not received\n");
        }
        next:
        pthread_mutex_lock(&ack_mutex);
        seq_num++;
        pthread_mutex_unlock(&ack_mutex);
    }
}

int get_routers_settings(int self_id){
    int  i, flag = TRUE;
    router aux;
    FILE *routers_settings = fopen("config/roteador.config", "r");
        for(i = 0, n_routers = 0; fscanf(routers_settings,"%d %d %s", &aux.id, &aux.port, aux.ip) != EOF; i++){
            routers = realloc (routers, sizeof(router) * ++(n_routers));

            routers[i].id = aux.id;
            routers[i].port = aux.port;
            sprintf((routers)[i].ip, "%s", aux.ip);

            if(aux.id == self_id){
                flag = FALSE;
                self_router = aux;
            }
        }
    fclose(routers_settings);

    if(flag){
        printf(">>[ERROR] Router with ID %d not configurated.\n", self_id);
        exit(1);
    }
    return(0);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf(">>[ERROR] %d arguments provided, expectating 1.\n", argc-1);
        exit(1);
    }
    int self_id = atoi(argv[1]);

    get_routers_settings(self_id);
    routing_table = realloc(routing_table, sizeof(routing_row)*(n_routers-1));
    create_routing_table(self_id, n_routers);

    if ( (mysocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf(">>[ERROR] socket()\n");
        exit(1);
    }

    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(self_router.port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

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
