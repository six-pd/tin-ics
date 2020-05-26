/*
 * Server Main Function
 *
 * Autorzy: Krzysztof Blankiewicz, Michal Urbanski, Tomasz Zaluska
 *
 * Data utworzenia: 10/04/2020
 */

#include "ClientHandling.h"
#include <errno.h>

#define TEMP_PORT 45456
pthread_mutex_t mutex;

int main(int argc, char **argv)
{
    typedef void* (*THREADFUNCPTR)(void*);

    srand(time(NULL));
    mutex = PTHREAD_MUTEX_INITIALIZER;

    int sock;
    socklen_t length;
    sockaddr_in6 serverAddr, clientAddr;
    char buf[1024];

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        std::cout << "Error opening socket" << std::endl;
        return -1;
    }

    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_addr = in6addr_any;
    serverAddr.sin6_port = htons(TEMP_PORT);
    if(bind(sock, (sockaddr*) &serverAddr, sizeof serverAddr) == 1)
    {
        std::cout << "Error binding stream socket" << std::endl;
        return -1;
    }

    length = sizeof(serverAddr);
    if (getsockname(sock,(sockaddr*) &serverAddr, &length) == -1)
    {
        std::cout << "Error getting socket name" << std::endl;
        return -1;
    }
    std::cout << "Socket port: " << ntohs(serverAddr.sin6_port) << std::endl << std::endl;
    
    do
    {
        pthread_mutex_lock(&mutex);
        int status = recvfrom(sock, buf, 1024, MSG_PEEK, (sockaddr*)&clientAddr, &length);
        pthread_mutex_unlock(&mutex);
        if(ClientHandling::findAddrInClients(clientAddr))
            continue;
            
        if (status < 0)
             std::cout << "Error on connecting, code " << errno << std::endl;
        else
        {
            pthread_t newThread;
            int newSock = sock;
            ClientHandling* newClient = new ClientHandling(newSock, clientAddr);
            memset(&clientAddr, 0, sizeof(clientAddr));
            memset(&length, 0, sizeof(length));
            pthread_create(&newThread, NULL, (THREADFUNCPTR) &ClientHandling::handleClient, newClient);
        }
    } while(true);
    
    return 0;
}
