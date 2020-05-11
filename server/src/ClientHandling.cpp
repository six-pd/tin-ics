#include "ClientHandling.h"
#include <string.h>
#include <string>
#include <time.h>

std::vector<ClientHandling*> ClientHandling::clientsList;

ClientHandling::ClientHandling(int newSocket, struct sockaddr_in6 newAddress)
{
	mySocket = newSocket;
	clientAddress = newAddress;
	clientsList.push_back(this);
	disconnectRequested = false;
	connect(mySocket, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
}


void ClientHandling::callProperMethod()
{
	int msgFlag = getFlagFromMsg();

	if(msgFlag == CL_CONNECTION_REQ)
	{
		startConnection();
		return;
	}
	if(msgFlag == CL_END_REQ)
	{
		endConnection();
		return;
	}
	if(msgFlag == CL_LIST_REQ)
	{
	sendClientsList();
		return;
	}
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

	int challenge = rand() % 1000;	//TODO normalny challenge
	
	sendString('0'+std::to_string(SRV_CHALLENGE_REQ)+';'+std::to_string(challenge)+';');

	if(!receiveData() || getFlagFromMsg() != CL_CHALLENGE_RESP)
	{
		protocolError(CL_CHALLENGE_RESP);
		return;
	}
	
	if(getIntArg(1) == challenge)
		sendString(std::to_string(SRV_CHALLENGE_ACC));
	else
		protocolError(CL_CHALLENGE_RESP);
	
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
		int newSSID = rand() % 1000;
		sendString('0'+std::to_string(SRV_NEW_SSID)+';'+std::to_string(newSSID)+';');
		ssid = newSSID;
	}
	//PREVIOUS_SSID_ACCEPT
	else
	{
		ssid = getIntArg(1);
		sendString(std::to_string(SRV_SSID_ACC)+';');
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
	std::string s = std::to_string(SRV_LIST_RESP);
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
	sendString(std::to_string(SRV_END_ACC));
	disconnectRequested = true;
	return;
}

void ClientHandling::sendString(std::string s)
{
	strcpy(bufOut, s.c_str());
	std::cout << bufOut << std::endl; 
	std::cout << send(mySocket, bufOut, sizeof(bufOut), 0);
	
	//write(mySocket, bufOut, 1024);
}

bool ClientHandling::receiveData()
{

	memset(bufIn, 0, sizeof(bufIn));
	int rval = recv(mySocket, bufIn, sizeof(bufIn), 0);
	std::cout << "received data: " << bufIn << ". Size: "<< rval << std::endl;
   	if (rval == -1)
    {
		std::cout << "Error reading stream message" << std::endl;
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
		return true;
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
	std::cout << "In thread " << pthread_self() << std::endl;
	
    int rval = 0;

    do{
		if(receiveData() && !disconnectRequested)
			callProperMethod();
		else
			break;
	}while(true);
	close(mySocket);
	// to jest turbo dziwne:
	delete this;	
	// ale znalazlem w internetach, ze tak mozna i nie widze innej opcji na usuwanie Client
	pthread_exit(NULL);
}

bool ClientHandling::findAddrInClients(struct sockaddr_in6 a)
{
	for(auto i: clientsList)
	{
		if((*((struct sockaddr_in*)&a)).sin_addr.s_addr == (*((struct sockaddr_in*)&(i->clientAddress))).sin_addr.s_addr)
			return true;
	}
	return false;
}