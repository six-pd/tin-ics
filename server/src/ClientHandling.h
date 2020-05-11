#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <vector>
#include <string>
#include "api.h"

class ClientHandling
{
    static std::vector<ClientHandling*> clientsList;

    int mySocket;
    struct sockaddr_in6 clientAddress;           // socket przypisany do klienta
    socklen_t clientAddressLen;
    std::string name;
    int ssid;
    bool disconnectRequested;
    char bufIn[BUF_SIZE];       // te dwa nie sa tutaj konieczne,
    char bufOut[BUF_SIZE];      // ale przydadza sie do obslugi bledow
                                // BUF_SIZE jest na razie w api.h

    
    void sendString(std::string s);
    bool receiveData();     // zwraca true jesli nie ma zadnych bledow
    
    // operacje na bufIn
    int getFlagFromMsg();       // pobiera flage z bufIn; flaga jest intem
    int getIntArg(int argNum);  // zerowy argument to flaga
    std::string getStringArg(int argNum);
    
    // obsluga bledow
    void protocolError(int flagExpected);
    void securityError();
    bool undefinedBehaviorError = false;
    void callProperMethod();    // decyduje ktora metode wywolac w zaleznosci od wiadomosci klienta
        // ponizsze trzy metody jak zostana wywolane, to przejmuja na chwile zarzadzenie sesja od handleClient
        // czyli wymieniaja kilka wiadomosci z klientem i dopiero sie koncza
    void startConnection();     
    void sendClientsList();
    void endConnection();

    // more specialized methods
    void sendAndCheckChallenge();
    void askForSSIDAndCheck();
    void getClientName();

    void removeFromClientsList();
public:
    ClientHandling(int newSocket, struct sockaddr_in6 newAddress);
    void* handleClient();   // wywoluje sie na poczatku, czeka na dane, wola callProperMethod

    static bool findAddrInClients(struct sockaddr_in6 a);
};
