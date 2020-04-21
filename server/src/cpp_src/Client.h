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

class Client{
    //int mySocket;
    //char buf[1024];

    void initialize(void* arg);
    void startConnection();
    void sendClientsList();
    void endConnection();
public:
    static void* handleClient(void* arg);
};