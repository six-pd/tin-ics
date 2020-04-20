#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define TRUE 1

void *function(void* arg)
{
	printf("Started connection\n");
	int mySocket = *((int*)arg);
	free(arg);
    char buf[1024];
    int rval;

    do{
   		memset(buf, 0, sizeof(buf));
   		if ((rval = read(mySocket,buf, 1024)) == -1)
    		perror("reading stream message");
    	if (rval == 0)
    		printf("Ending connection\n");
    	else
        	printf("-->%s\n", buf);
	}while(rval != 0);
	close(mySocket);
	pthread_exit(NULL);
}

int  main(int argc, char **argv)
{
    int sock, length;
    struct sockaddr_in6 server;
    int msgsock;

    sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("opening stream socket");
        exit(1);
    }
/* dowiaz adres do gniazda */

    server.sin6_family = AF_INET6;
    server.sin6_addr = in6addr_any;
    server.sin6_port = 0;
    if (bind(sock, (struct sockaddr *) &server, sizeof server)
        == -1) {
        perror("binding stream socket");
        exit(1);
    }
    /* wydrukuj na konsoli przydzielony port */
    length = sizeof( server);
    if (getsockname(sock,(struct sockaddr *) &server,&length)
         == -1) {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port #%d\n", ntohs(server.sin6_port));
    /* zacznij przyjmowaæ polaczenia... */
    listen(sock, 5);
    
do {
	pthread_t newThread;
        msgsock = accept(sock,(struct sockaddr *) 0,(int *) 0);
        if (msgsock == -1 )
             perror("accept");
        else {
		int* newsock = malloc(sizeof(int));
		*newsock = msgsock;
		printf("Starting thread\n");
		pthread_create(&newThread, NULL, (void*) &function, newsock);
        };
    } while(TRUE);
    /*
     * gniazdo sock nie zostanie nigdy zamkniete jawnie,
     * jednak wszystkie deskryptory zostana zamkniete gdy proces 
     * zostanie zakonczony (np w wyniku wystapienia sygnalu) 
     */
	
     exit(0);
}
