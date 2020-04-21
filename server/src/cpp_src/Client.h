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

class Client;

struct threadSocketAssignment{
    pthread_t thread;
    int socket;
    Client* client;
    threadSocketAssignment(pthread_t p, int i, Client* c);
    ~threadSocketAssignment();
};

class Client{
    
    static std::vector<threadSocketAssignment> socketsTab;      // tablica socketow
    int mySocket = 57;       // moj socket
    char buf[1024];     // tu bedzie zapisywane co przyjdzie od klientat

    void startConnection();
    void sendClientsList();
    void endConnection();
public:
    Client(int newSocket);
    static void addToSocketsTab(pthread_t p, int i, Client* c);
    static void cleanTab();
    static void printTab();
    void* handleClient();
};