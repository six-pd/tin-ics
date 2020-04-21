#include "Client.h"


void Client::initialize(void* arg)
{

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

void* Client::handleClient(void* arg)
{
    /*
    initialize(arg);
    
    int rval;
    do{
   		std::memset(buf, 0, sizeof(buf));
   		if ((rval = std::read(mySocket,buf, 1024)) == -1)
    		std::cout<<"error reading stream message" << std::endl;
    	if (rval == 0)
    		std::cout<<"Ending connection" << std::endl;

	}while(rval != 0);
    */
    std::cout << "Started connection" << std::endl;
	int mySocket = *((int*)arg);
	delete(arg);
    char buf[1024];
    int rval;

    do{
   		memset(buf, 0, sizeof(buf));
   		if ((rval = read(mySocket,buf, 1024)) == -1)
    		std::cout << "reading stream message" << std::endl;
    	if (rval == 0)
    		std::cout <<"Ending connection\n" << std::endl;
    	else
        	std::cout << buf << std::endl;
	}while(rval != 0);
	close(mySocket);
	pthread_exit(NULL);
}