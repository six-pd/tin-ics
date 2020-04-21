#include"Client.h"

int  main(int argc, char **argv)
{
    typedef void * (*THREADFUNCPTR)(void *);

    int sock;
    socklen_t length;
    struct sockaddr_in6 server;
    int msgsock;

    sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
       std::cout << "opening stream socket" << std::endl;
        return -1;
    }
/* dowiaz adres do gniazda */

    server.sin6_family = AF_INET6;
    server.sin6_addr = in6addr_any;
    server.sin6_port = 0;
    if (bind(sock, (struct sockaddr *) &server, sizeof server)
        == -1) {
        std::cout << "binding stream socket" << std::endl;
        return -1;
    }
    /* wydrukuj na konsoli przydzielony port */
    length = sizeof( server);
    if (getsockname(sock,(struct sockaddr *) &server, &length)
         == -1) {
        std::cout << "getting socket name" << std::endl;
        return -1;
    }
    std::cout << "Socket port " << ntohs(server.sin6_port) << std::endl;
    /* zacznij przyjmowa� polaczenia... */
    listen(sock, 5);
    
    do {
        msgsock = accept(sock, (struct sockaddr *) 0,(socklen_t *) 0);
        if (msgsock == -1 )
             std::cout << "Error on accepting" << std::endl;
        else 
        {
            pthread_t newThread;
            int newSock = msgsock;
            Client* newClient = new Client(msgsock);

            pthread_create(&newThread, NULL, (THREADFUNCPTR) &Client::handleClient, newClient);
        }
    } while(true);
    
    return 0;
}