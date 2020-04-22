#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include<iostream>
#include<cstring>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <vector>

class Client
{    
    static std::vector<Client*> clientsList;
    int mySocket;       // moj socket
    char buf[1024];     // tu bedzie zapisywane co przyjdzie od klientat

    void callProperMethod();
    void startConnection();
    void sendClientsList();
    void endConnection();
public:
    Client(int newSocket);
    void* handleClient();
};