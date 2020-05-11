#include"ClientHandling.h"
#include<errno.h>

int  main(int argc, char **argv)
{
    typedef void * (*THREADFUNCPTR)(void *);
   	srand(time(NULL));

    int sock;
    socklen_t length;
    struct sockaddr_in6 serverAddr, clientAddr;
    char buf[1024];

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1) {
       std::cout << "opening stream socket" << std::endl;
        return -1;
    }
/* dowiaz adres do gniazda */

    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_addr = in6addr_any;
    serverAddr.sin6_port = 0;
    if (bind(sock, (struct sockaddr *) &serverAddr, sizeof serverAddr)
        == -1) {
        std::cout << "binding stream socket" << std::endl;
        return -1;
    }
    /* wydrukuj na konsoli przydzielony port */
    length = sizeof( serverAddr);
    if (getsockname(sock,(struct sockaddr *) &serverAddr, &length)
         == -1) {
        std::cout << "getting socket name" << std::endl;
        return -1;
    }
    std::cout << "Socket port " << ntohs(serverAddr.sin6_port) << std::endl;
    /* zacznij przyjmowa� polaczenia... */
    //listen(sock, 5);
    
    do {
        int status = recvfrom(sock, buf, 1024, MSG_PEEK, (struct sockaddr*)&clientAddr, &length);
        if(ClientHandling::findAddrInClients(clientAddr))
            continue;
            
        if (status < 0 )
             std::cout << "Error on connecting" << " " << errno << std::endl;
        else
        {
            pthread_t newThread;
            int newSock = sock;
            ClientHandling* newClient = new ClientHandling(newSock, clientAddr);

            pthread_create(&newThread, NULL, (THREADFUNCPTR) &ClientHandling::handleClient, newClient);
        }
    } while(true);
    
    return 0;
}