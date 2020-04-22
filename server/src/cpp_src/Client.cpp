#include "Client.h"
#include <string.h>

std::vector<Client*> Client::clientsList;

Client::Client(int newSocket)
{
	mySocket = newSocket;
	clientsList.push_back(this);
}

void Client::callProperMethod()
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

void Client::startConnection()
{
	std::string stringToSend;

	// Server: 	SRV_CHALLENGE_REQ 
	stringToSend = std::to_string(SRV_CHALLENGE_REQ);
	sendString(stringToSend);
 	// Client: 	CL_CHALLENGE_RESP 
	if(!receiveData() || getFlagFromMsg() != CL_CHALLENGE_RESP)
	{
		protocolError(CL_CHALLENGE_RESP);
		return;
	}
 	// Server: 	SRV_CHALLENGE_ACC 
					// sparwdzic challenge
	stringToSend = std::to_string(SRV_CHALLENGE_ACC);
	sendString(stringToSend);
 	// Client: 	CL_SSID_REQ 
	if(!receiveData() || getFlagFromMsg() != CL_SSID_REQ)
	{
		protocolError(CL_SSID_REQ);
		return;
	}
 	// Server: 	SRV_NEW_SSID 
	if(getIntArg(1) == 0)
	{
		stringToSend = std::to_string(SRV_NEW_SSID);
		sendString(stringToSend);
	}
	// Server: 	SRV_SSID_ACC
	else
	{
		stringToSend = std::to_string(SRV_SSID_ACC);
		sendString(stringToSend);
	}
 	// Client: 	CL_NAME
	if(!receiveData() || getFlagFromMsg() != CL_NAME)
	{
		protocolError(CL_NAME);
		return;
	}
 	// Server: 	SRV_NAME_ACC
					// zapisz imie
	stringToSend = std::to_string(SRV_CHALLENGE_ACC);
	sendString(stringToSend);
}

void Client::sendClientsList()
{

}

void Client::endConnection()
{

}

void Client::sendString(std::string s)
{
	strcpy(bufOut, s.c_str());
	write(mySocket, bufOut, 1024);
}

bool Client::receiveData()
{
	int rval;
	memset(bufIn, 0, sizeof(bufIn));
   	if ((rval = read(mySocket, bufIn, 1024)) == -1)
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

int Client::getFlagFromMsg()
{
	return getIntArg(0);
}

int Client::getIntArg(int argNum)
{
	int i = 0;
	int currentArg = 0;
	while(currentArg != argNum)
	{
		if(bufIn[i] == ';')
			++currentArg;
		++i;
		if(i >= BUF_SIZE)
		{
			sequrityError();
			return -1;
		}
	}
	// teraz i wskazuje na pierwszy element szukanego argumentu
	std::string targetArgument;
	while(bufIn[i] != ';')
	{
		targetArgument += bufIn[i];
		++i;
		if(i >= BUF_SIZE)
		{
			sequrityError();
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

std::string Client::getStringArg(int argNum)
{
	int i = 0;
	int currentArg = 0;
	while(currentArg != argNum)
	{
		if(bufIn[i] == ';')
			++currentArg;
		++i;
		if(i >= BUF_SIZE)
		{
			sequrityError();
			return "";
		}
	}
	// teraz i wskazuje na pierwszy element szukanego argumentu
	std::string targetArgument;
	while(bufIn[i] != ';')
	{
		targetArgument += bufIn[i];
		++i;
		if(i >= BUF_SIZE)
		{
			sequrityError();
			return "";
		}
	}
	return targetArgument;
}

void Client::protocolError(int flagExpected)
{
	std::cout << "Protocol error, " << flagExpected << " with proper agruments expected. " << 
			"Aborting sequence." << std::endl;
}

void Client::sequrityError()
{
	std::cout << "Wrong sent data - possible threat" << std::endl;
	// zrobic klientowi jakas krzywde
}

void* Client::handleClient()
{
	std::cout << "In thread " << pthread_self() << std::endl;
	
    int rval = 0;

    do{
		if(receiveData())
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
