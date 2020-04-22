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

class Client
{
    static std::vector<Client*> clientsList;
    int mySocket;           // moj socket
    std::string name;
    int ssid;
    char bufIn[BUF_SIZE];       // te dwa nie sa tutaj konieczne,
    char bufOut[BUF_SIZE];      // ale przydadza sie do obslugi bledow
                                // BUF_SIZE jest na razie w api.h
    

    void callProperMethod();    // decyduje ktora metode wywolac w zaleznosci od wiadomosci klienta
    void sendString(std::string s);
    bool receiveData();     // zwraca true jesli nie ma zadnych bledow
    int getFlagFromMsg();   // pobiera flage z bufIn, 
                            // zakladam, ze flagi sa dwucyfrowe
    int getIntArg(int argNum);  // zerowy argument to flaga
    std::string getStringArg(int argNum);
    void protocolError(int flagExpected);
    void sequrityError();

        // ponizsze trzy metody jak zostana wywolane, to przejmuja na chwile zarzadzenie sesja od handleClient
        // czyli wymieniaja kilka wiadomosci z klientem i dopiero sie koncza
    void startConnection();     
    void sendClientsList();
    void endConnection();

public:
    Client(int newSocket);
    void* handleClient();   // wywoluje sie na poczatku, czeka na dane, wola callProperMethod
};