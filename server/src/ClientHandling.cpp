#include "ClientHandling.h"
#include <string.h>
#include <string>
#include <time.h>
#include <algorithm>

extern pthread_mutex_t mutex;

std::vector<ClientHandling*> ClientHandling::clientsList;

ClientHandling::ClientHandling(int newSocket, sockaddr_in6 newAddress)
{
	mySocket = newSocket;
	clientAddress = newAddress;
	clientAddressLen = sizeof(clientAddress);
	clientsList.push_back(this);
	disconnectRequested = false;
	//connect(mySocket, (sockaddr*)&clientAddress, sizeof(clientAddress));
}


void ClientHandling::callProperMethod()
{
	int msgFlag = getFlagFromMsg();

	if(msgFlag == CL_CONNECTION_REQ)
	{
		startConnection();
		return;
	}
	else if(msgFlag == CL_END_REQ)
	{
		endConnection();
		return;
	}
	else if(msgFlag == CL_LIST_REQ)
	{
		sendClientsList();
		return;
	}
	else 
		disconnectRequested = true;
}

void ClientHandling::startConnection()
{

	sendAndCheckChallenge();
	askForSSIDAndCheck();
	getClientName();

	std::cout << "Client SSID: " << ssid << std::endl;
	std::cout << "Client name: " << name << std::endl;
}

void ClientHandling::sendAndCheckChallenge()
{

	int challenge = (rand() % 900) + 100;	//TODO normalny challenge
	
	sendString('0'+std::to_string(SRV_CHALLENGE_REQ)+';'+std::to_string(challenge)+';');

	if(!receiveData() || getFlagFromMsg() != CL_CHALLENGE_RESP)
	{
		protocolError(CL_CHALLENGE_RESP);
		return;
	}
	if(getIntArg(1) == challenge)
		sendString('0'+std::to_string(SRV_CHALLENGE_ACC)+';'+'0'+';');
	else
	{
		sendString('0'+std::to_string(SRV_CHALLENGE_ACC)+';'+'1'+';');
		protocolError(CL_CHALLENGE_RESP);
	}
}

void ClientHandling::askForSSIDAndCheck()
{
	if(!receiveData() || getFlagFromMsg() != CL_SSID_REQ)
	{
		protocolError(CL_SSID_REQ);
		return;
	}
	//NEW_SSID:
	if(getIntArg(1) == 0)
	{
		std::cout << "choosing new id" << std::endl;
		int newSSID = (rand() % 900) + 100;
		sendString('0'+std::to_string(SRV_NEW_SSID)+';'+std::to_string(newSSID)+';');
		ssid = newSSID;
	}
	//PREVIOUS_SSID_ACCEPT
	else
	{
		std::cout << "reading id from file" << std::endl;
		ssid = getIntArg(1);
		sendString('0'+std::to_string(SRV_SSID_ACC)+';');
	}
}

void ClientHandling::getClientName()
{
	
	if(!receiveData() || getFlagFromMsg() != CL_NAME)
	{
		protocolError(CL_NAME);
		return;
	}

	name = getStringArg(1);
	sendString('0'+std::to_string(SRV_NAME_ACC)+';');
}


void ClientHandling::sendClientsList()
{
	std::string s = std::to_string(SRV_LIST_RESP) + ';';
	for (auto a: clientsList)
	{
		s += a->name;
		s += ';';
	}
	sendString(s);
	
	if(!receiveData() || getFlagFromMsg() != CL_LIST_ACC)
	{
		protocolError(CL_LIST_ACC);
		return;
	}
	return;
}

void ClientHandling::endConnection()
{
	sendString(std::to_string(SRV_END_ACC)+';');
	disconnectRequested = true;
	return;
}

void ClientHandling::sendString(std::string s)
{
	strcpy(bufOut, s.c_str());
	//pthread_mutex_lock(&mutex);
	sendto(mySocket, bufOut, s.length(), 0, (sockaddr*)&clientAddress, sizeof(clientAddress));
	//pthread_mutex_unlock(&mutex);
	//write(mySocket, bufOut, 1024);
}

bool ClientHandling::receiveData()
{
	socklen_t len;
	sockaddr_in6 newAddress;
	len = sizeof(newAddress);
	int rval;
	memset(bufIn, 0, sizeof(bufIn));
	pthread_mutex_lock(&mutex);
	recvfrom(mySocket, bufIn, sizeof(bufIn), MSG_PEEK, (sockaddr*)&newAddress, &len);
	if((*((sockaddr_in*)&newAddress)).sin_addr.s_addr == (*((sockaddr_in*)&clientAddress)).sin_addr.s_addr)
	{
		rval = recvfrom(mySocket, bufIn, sizeof(bufIn), 0, (sockaddr*)&clientAddress, &clientAddressLen);
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}
	pthread_mutex_unlock(&mutex);
   	if (rval == -1)
    {
		std::cout << "Error reading stream message " << errno << std::endl;
		return false;
	}
    else if (rval == 0)
    {
		std::cout << "Ending connection\n" << std::endl;
		return false;
	}
	else if (rval > 1024)
	{
		std::cout << "Buffer overflow" << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}

int ClientHandling::getFlagFromMsg()
{
	return getIntArg(0);
}

int ClientHandling::getIntArg(int argNum)	// TODO nie sprawdzam, czy liczba jest liczba
{
	int i = 0;
	int currentArg = 0;
	while(currentArg != argNum && bufIn[i] != '\0')
	{
		if(bufIn[i] == ';')
			++currentArg;
		++i;
		if(i >= BUF_SIZE)
		{
			securityError();
			return -1;
		}
	}
	
	// teraz i wskazuje na pierwszy element szukanego argumentu
	std::string targetArgument;
	while(bufIn[i] != ';' && bufIn[i] != '\0')
	{
		targetArgument += bufIn[i];
		++i;
		if(i >= BUF_SIZE)
		{
			securityError();
			return -1;
		}
	}

	// mamy szukany argument jako string
	int result = 0;
	for(i = 0; i < targetArgument.size(); ++i)
	{
		result = result*10 + (targetArgument[i] - '0');
	}
	return result;
}

std::string ClientHandling::getStringArg(int argNum)
{
	int i = 0;
	int currentArg = 0;
	while(currentArg != argNum && bufIn[i] != '\0')
	{
		if(bufIn[i] == ';')
			++currentArg;
		++i;
		if(i >= BUF_SIZE)
		{
			securityError();
			return "";
		}
	}
	// teraz i wskazuje na pierwszy element szukanego argumentu
	std::string targetArgument;
	while(bufIn[i] != ';' && bufIn[i] != '\0')
	{
		targetArgument += bufIn[i];
		++i;
		if(i >= BUF_SIZE)
		{
			securityError();
			return "";
		}
	}
	return targetArgument;
}

void ClientHandling::protocolError(int flagExpected)
{
	std::cout << "Protocol error, " << flagExpected << " with proper agruments expected. " << 
			"Aborting sequence." << std::endl;
	undefinedBehaviorError = true;
}

void ClientHandling::securityError()
{
	std::cout << "Wrong sent data - possible threat" << std::endl;
	// zrobic klientowi jakas krzywde
	undefinedBehaviorError = true;
}

void* ClientHandling::handleClient()
{
	std::cout << "Creating new thread no. " << pthread_self() << std::endl;
	
    int rval = 0;

    do{
		if(receiveData())
			callProperMethod();
		else
			break;
	}while(!disconnectRequested);
	std::cout << "Exiting thread" << std::endl;
	removeFromClientsList();
	// to jest turbo dziwne:
	delete this;
	// ale znalazlem w internetach, ze tak mozna i nie widze innej opcji na usuwanie Client
	pthread_exit(NULL);
}

bool ClientHandling::findAddrInClients(sockaddr_in6 a)
{
	for(auto i: clientsList)
	{
		if((*((sockaddr_in*)&a)).sin_addr.s_addr == (*((sockaddr_in*)&(i->clientAddress))).sin_addr.s_addr)
		{	
			return true;
		}
	}
	return false;
}

void ClientHandling::removeFromClientsList()
{
	for (auto it = clientsList.begin(); it != clientsList.end(); ) {
        if ((*it)->name == name){
            clientsList.erase(it);
        }
    }
	/*int i = 0;
	for(; i < clientsList.size(); ++i)
	{
		if(clientsList[i] == this)
		std::cout  << "mam " << i << std::endl;
	}
	clientsList.erase(std::find(clientsList.begin(), clientsList.end(), this));
	clientsList.*/
}
