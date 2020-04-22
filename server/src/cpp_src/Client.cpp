#include "Client.h"
#include <string.h>
#include <time.h>

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
	sendAndCheckChellenge();
	askForSSIDAndCheck();
	getClientName();

	std::cout << "Client SSID: " << ssid << std::endl;
	std::cout << "Client name: " << name << std::endl;
}

void Client::sendAndCheckChellenge()
{
	int challenge = rand() % 1000;	//TODO normalny challenge
	
	sendString(std::to_string(SRV_CHALLENGE_REQ)+';'+std::to_string(challenge)+';');

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

void Client::askForSSIDAndCheck()
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
		sendString(std::to_string(SRV_NEW_SSID)+';'+std::to_string(newSSID)+';');
		ssid = newSSID;
	}
	//PREVIOUS_SSID_ACCEPT
	else
	{
		ssid = getIntArg(1);
		sendString(std::to_string(SRV_SSID_ACC)+';');
	}
}

void Client::getClientName()
{
	if(!receiveData() || getFlagFromMsg() != CL_NAME)
	{
		protocolError(CL_NAME);
		return;
	}

	name = getStringArg(1);
	sendString(std::to_string(SRV_NAME_ACC)+';');
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

int Client::getIntArg(int argNum)	// TODO nie sprawdzam, czy liczba jest liczba
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
	undefinedBehaviorError = true;
}

void Client::sequrityError()
{
	std::cout << "Wrong sent data - possible threat" << std::endl;
	// zrobic klientowi jakas krzywde
	undefinedBehaviorError = true;
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