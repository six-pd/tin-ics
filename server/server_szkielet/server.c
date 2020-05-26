/*
 * Szablon komunikacji UDP po IPv6 - serwer
 *
 * Autorzy: Tomasz Zaluska, Krzysztof Blankiewicz
 *
 * Data utworzenia: 10/05/2020
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define TRUE 1
#define TEXT "Data from new thread"

int sock;

void *function(void* arg)
{   char buf;
    int rval;
    struct sockaddr_in6 my_sender = *((struct sockaddr_in6*) arg);
    
    srand(time(NULL));
    printf("Started new thread for client, thread id: %d\n", rand()%1000);
    
    free(arg);
    
    printf("addr: %d\n", inet_ntoa(my_sender.sin6_addr));
    sendto(sock, TEXT, sizeof(TEXT), 0, (struct sockaddr_in6 *) &my_sender, sizeof(my_sender));
    pthread_exit(NULL);
    
}



void main(void)
{
    int length;
    struct sockaddr_in6 name;
    char buf[1024];
    
    
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    
    name.sin6_family = AF_INET6;
    name.sin6_addr = in6addr_any;
    name.sin6_port = 0;
    if(bind(sock, (struct sockaddr *) &name, sizeof(name)) == -1){
        perror("binding datagram socket");
        exit(1);
    }
    length = sizeof(name);
    if(getsockname(sock, (struct sockaddr *) &name, &length) == -1){
        perror("getting sock name");
        exit(1);
    }
    
    printf("Socket port #%d\n", ntohs(name.sin6_port));
    do {
        struct sockaddr_in6 sender_addr;
        int sender_len = sizeof(struct sockaddr_in6);
        pthread_t newThread;
        
        if(recvfrom(sock, buf, 1024, 0, &sender_addr, &sender_len) == -1){
            perror("reciving datagram packet");
            exit(2);
        }
        struct sockaddr_in6 * new_sender = malloc(sizeof(struct sockaddr_in6));
        *new_sender = sender_addr;
        printf("Starting thread\n");
        pthread_create(&newThread, NULL, (void*) & function, new_sender);
    } while(TRUE);

    //printf("-->%s\n", buf);
    //printf("addr: %d\n", inet_ntoa(sender_addr.sin_addr));
    close(sock);
    exit(0);
}
