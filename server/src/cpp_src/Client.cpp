#include "Client.h"

std::vector<Client*> Client::clientsList;

Client::Client(int newSocket)
{
	mySocket = newSocket;
	clientsList.push_back(this);
}


void Client::startConnection()
{

}

void Client::sendClientsList()
{

}

void Client::endConnection()
{

}

void* Client::handleClient()
{
	std::cout << "In thread " << pthread_self() << std::endl;
	
    int rval = 0;

    do{
   		memset(buf, 0, sizeof(buf));
   		if ((rval = read(mySocket,buf, 1024)) == -1)
    		std::cout << "Error reading stream message" << std::endl;
    	if (rval == 0)
    		std::cout <<"Ending connection\n" << std::endl;
    	else
        	std::cout << buf << std::endl;
	}while(rval != 0);
	close(mySocket);
	// to jest turbo dziwne:
	delete this;	
	// ale znalazlem w internetach, ze tak mozna i nie widze innej opcji na usuwanie Client
	pthread_exit(NULL);
}
