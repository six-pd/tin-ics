#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DATA "Half a league, half a league . . ."

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in6 server;
    struct hostent *hp;
    char buf[1024];

    /* Create socket. */
    sock = socket( AF_INET6, SOCK_STREAM, 0 );
    if (sock == -1) {
        perror("opening stream socket");
        exit(1);
    }

    /* uzyskajmy adres IP z nazwy . */
    server.sin6_family = AF_INET6;
    hp = gethostbyname2(argv[1], AF_INET6 );

/* hostbyname zwraca strukture zawierajaca adres danego hosta */
    if (hp == (struct hostent *) 0) {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    memcpy((char *) &server.sin6_addr, (char *) hp->h_addr,
        hp->h_length);
    server.sin6_port = htons(atoi( argv[2]));
    if (connect(sock, (struct sockaddr *) &server, sizeof server)
        == -1) {
        perror("connecting stream socket");
        exit(1);
    }
    if (write( sock, DATA, sizeof DATA ) == -1)

        perror("writing on stream socket");
    sleep(5);
    if (write( sock, DATA, sizeof DATA ) == -1)

        perror("writing on stream socket");

    close(sock);
    exit(0);}
