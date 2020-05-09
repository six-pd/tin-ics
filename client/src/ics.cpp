#include "ics.h"
ics_server::ics_server()
{
	/*
	 * Tworzymy gniazdo
	 */
	sock = socket(AF_INET6, SOCK_DGRAM,0);
	if(sock == -1)
		throw "Failed to create IPv6 socket!\n";
	/*
	 * IPv6
	 */
	server.sin6_family = AF_INET6;
}

int ics_server::ics_recv(int len, char* flag, int tries = 20){
	char temp[BUF_SIZE];
	for(int i = 0;i++;i < tries){
		if(recv(sock, temp, len, 0) != len){
			send(sock, msg, strlen(msg), 0);
			continue;
		}
		if(strncmp(temp, flag, 2) != 0){
			send(sock, msg, strlen(msg), 0);
			continue;
		}else{
			strcpy(buf, temp + 3);
			return 0;
		}
	}
	return -1;
}

int ics_server::ics_handshake()
{	
	//char ch[CH_LEN];
	char* pass;
	char* ssid;
	std::fstream ssid_file;

	msg = CL_CONNECTION_REQ;
	send(sock, msg, strlen(msg), 0);

	if(ics_recv(3+CH_LEN, SRV_CHALLENGE_REQ) != 0)
		return -2;

	strcpy(msg, CL_CHALLENGE_RESP);
	msg = strcat(msg, ";\0");
	pass = "password";
	//msg = strcat(msg, ics_auth(pass, ch));
	send(sock, msg, strlen(msg), 0);

	if(ics_recv(4, SRV_CHALLENGE_ACC) != 0)
		return -4;

	if(strcmp(buf, "0") != 0)
		return -3; //incorrect password
	
	strcpy(msg, CL_SSID_REQ);
	msg = strcat(msg, ";\0");

	ssid = "\0";
	ssid_file.open(strcat(dirpath, "ssid"));
	if(!ssid_file.is_open())
		return -5;
	ssid_file >> ssid;
	if(strlen(ssid) != 3)
		ssid = "0";

	msg = strcat(msg, ssid);
	send(sock, msg, strlen(msg), 0);

	if(ssid == "0"){
		if(ics_recv(6, SRV_NEW_SSID) != 0)
			return -6;
		ssid_file.write(buf, 3);
	}else{
		if(ics_recv(2, SRV_SSID_ACC) != 0)
			return -7;
	}
	strcpy(msg, CL_NAME);
	msg = strcat(msg, ";\0");
	
	char* name = "nickname";
	msg = strcat(msg, name);

	if(ics_recv(2, SRV_NAME_ACC)!= 0)
		return -9;
	return 0;
}
int ics_server::ics_connect(char* address, int port)
{
	/*
	 * Uzyskujemy IP z char* address, w formie d:d:d:d:d:d:d:d
	 */
	int e = inet_pton(AF_INET6, address, (void*) &server.sin6_addr);
	if(e <= 0){
		if(e == 0)
			throw "Not a valid IP address";
		else
			throw "AF not supported";
	}
	server.sin6_port = htons(port);
	if(connect(sock, (struct sockaddr*) &server, sizeof(server)) == -1)
		throw "Error while connecting socket.\n";

	int hs = ics_handshake();
	if(hs < 0){
		printf("Error code: %d", hs*(-1));
		throw "Handshake error";
	}

	return 0;
}

int ics_server::ics_clist(){
	return 0;
}

int ics_server::ics_disconnect(){
	return 0;
}
