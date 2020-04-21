#include "ics.h"
ics_server::ics_server()
{
	/*
	 * Tworzymy gniazdo
	 */
	this->sock = socket(AF_INET6, SOCK_DGRAM,0);
	if(sock == -1)
		throw "Failed to create IPv6 socket!\n";
	/*
	 * IPv6
	 */
	this->server.sin6_family = AF_INET6;
}

int ics_server::ics_connect(char* address, int port)
{
	/*
	 * Uzyskujemy IP z nazwy
	 */
	this->hp = gethostbyname2(address, AF_INET6);
	if(this->hp == (struct hostent*) 0)
		throw "Unknown host!\n";
	memcpy((char*) &(this->server.sin6_addr), (char*) this->hp->h_addr, this->hp->h_length);
	this->server.sin6_port = htons(port);
	if(connect(this->sock, (struct sockaddr*) &(this->server), sizeof (this->server)) == -1)
		throw "Error while connecting socket.\n";
	return 0;
}
