#include "Client.h"

threadSocketAssignment::threadSocketAssignment(pthread_t p, int i, Client* c){
    thread = p;
    socket = i;
    client = c;
}

threadSocketAssignment::~threadSocketAssignment(){
    //delete thread;
    //delete socket;
    delete client;
}

std::vector<threadSocketAssignment> Client::socketsTab;

Client::Client(int newSocket)
{
	std::cout << mySocket << std::endl;
	mySocket = newSocket;
	std::cout << mySocket << std::endl;
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
	std::cout << "In thread" << pthread_self() << std::endl;
	std::cout << "This" << this << std::endl;

	//int mySocket = *((int*)arg);
	//delete(arg);
    //char buf[1024];
    int rval = 0;

    do{
		std::cout << "Moj socket: " << mySocket << std::endl;
   		memset(buf, 0, sizeof(buf));
   		if ((rval = read(mySocket,buf, 1024)) == -1)
    		std::cout << "Error reading stream message" << std::endl;
    	if (rval == 0)
    		std::cout <<"Ending connection\n" << std::endl;
    	else
        	std::cout << "Mam dane " << buf[7] << std::endl;
	}while(rval != 0);
	close(mySocket);
	pthread_exit(NULL);
}

void Client::cleanTab()
{
	socketsTab.clear();
}

void Client::printTab()
{
	std::cout << socketsTab.size() << std::endl;
	for(auto i:socketsTab)
	{
		std::cout << i.thread << " " << i.socket << std::endl;
	}
	std::cout << std::endl;
}