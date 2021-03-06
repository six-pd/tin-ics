/*
 * Szablon komunikacji UPD po IPv6 - klient
 *
 * Autor: Tomasz Zaluska, Krzysztof Blankiewicz
 *
 * Data utworzenia: 10/05/2020
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA "The sea is calm, the tide is full . . . "

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in name;
    struct hostent *hp;
    
    char buf[1024];
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1){
        perror("opening datagram socket");
        exit(1);
    }
    hp = gethostbyname(argv[1]);
    if(hp == (struct hostent *) 0) {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    memcpy((char *) &name.sin_addr, (char *) hp->h_addr, hp->h_length);
    name.sin_family = AF_INET;
    name.sin_port = htons(atoi(argv[2]));
    if(sendto(sock, DATA, sizeof(DATA), 0, (struct sockaddr *) &name, sizeof(name)) == -1)
        perror("sending datagram message");
    sleep(1);
    recv(sock, buf, 1024, 0);
    printf("-->%s\n", buf);
    close(sock);
    exit(0);
}
