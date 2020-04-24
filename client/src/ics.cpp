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

int ics_server::ics_handshake()
{	

	msg = CL_CONNECTION_REQ;
	send(sock, msg, strlen(msg), 0);
	for(;;){
		if(recv(sock, buf, 2 + CH_LEN, 0) != 2 + CH_LEN){
			//send(sock, msg, strlen(msg), 0);
			continue;
		}
		if(strncmp(buf, SRV_CHALLENGE_REQ, 2) != 0){
			//send(sock, msg, strlen(msg), 0);
			continue;
		}
	}
}
int ics_server::ics_connect(char* address, int port)
{
	/*
	 * Uzyskujemy IP z char* address, w formie d:d:d:d:d:d:d:d
	 */
	int e = inet_pton(AF_INET6, address, (void*) &(this->server.sin6_addr));
	if(e <= 0){
		if(e == 0)
			throw "Not a valid IP address";
		else
			throw "AF not supported";
	}
	this->server.sin6_port = htons(port);
	if(connect(this->sock, (struct sockaddr*) &(this->server), sizeof (this->server)) == -1)
		throw "Error while connecting socket.\n";
	return 0;
}

