#include "ics.h"
ics_server::ics_server()
{
	/*
	 * Tworzymy gniazdo
	 */
	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if(sock == -1)
		throw "Failed to create IPv6 socket!\n";
	/*
	 * IPv6
	 */
	server.sin6_family = AF_INET6;
	client.sin6_family = AF_INET6;
	client.sin6_addr = in6addr_any;
	client.sin6_port = 0;

	if(bind(sock, (struct sockaddr*) &client, sizeof(client)) == -1)
		throw "Failed to bind IPv6 socket!\n";
}

int ics_server::ics_recv(int len, std::string flag, int tries)
{
	char temp[len+1];
	for(int i = 0;i <= tries;++i){
		int numbytes = recv(sock, temp, len, 0);
		if (numbytes == -1){
			std::cout << "Recv function error: " << errno << '\n';
			return -2;
		}
		if(numbytes > len){
			send(sock, msg.c_str(), msg.length(), 0);
			continue;
		}
		temp[len] = '\0';
		if(strncmp(temp, flag.c_str(), 2) != 0)
		{
			send(sock, msg.c_str(), msg.length(), 0);
			continue;
		}
		else
		{
			std::cout << "Message received by client: " << temp << '\n';
			buf = temp + 3;
			buf.pop_back(); //ends with ';'
			return 0;
		}
	}
	return -1;
}

int ics_server::ics_handshake()
{	
	//char ch[CH_LEN];
	std::string pass;
	std::string ssid;
	std::fstream ssid_file;

	msg = CL_CONNECTION_REQ;
	msg.append(";");
	//std::cout << msg << '\n';
	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(3 + CH_LEN + 1, SRV_CHALLENGE_REQ) != 0)
		return -2;

	msg = CL_CHALLENGE_RESP;
	msg.append(";");

	pass = "password;";
	msg.append(buf);
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(5, SRV_CHALLENGE_ACC) != 0)
		return -4;

	if(buf != "0")
		return -3; //incorrect password
	
	msg = CL_SSID_REQ;
	msg.append(";");

	ssid_file.open(dirpath + "ssid");
	if(!ssid_file.is_open())
		return -5;
	ssid_file >> ssid;
	if(ssid.length() != 3)
		ssid = "0";

	msg.append(ssid);
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);

	if(ssid == "0"){
		if(ics_recv(7, SRV_NEW_SSID) != 0)
			return -6;
		ssid_file.write(buf.c_str(), 3);
	}else{
		if(ics_recv(3, SRV_SSID_ACC) != 0)
			return -7;
	}
	msg = CL_NAME;
	msg.append(";");
	
	std::string name = "nickname";
	name.append(";");
	msg.append(name);

	send(sock, msg.c_str(), msg.length(), 0);
	
	if(ics_recv(3, SRV_NAME_ACC) != 0)
		return -9;
	return 0;
}
int ics_server::ics_connect()
{
	/*
	 * Uzyskujemy IP z char* address, w formie d:d:d:d:d:d:d:d
	 */
	int e = inet_pton(AF_INET6, this->addr.c_str(), (void*) &server.sin6_addr);
	if(e <= 0){
		if(e == 0){
			throw "Not a valid IP address\n";
			return -1;
		}
		else{
			throw "AF not supported\n";
			return -2;
		}
	}
	server.sin6_port = htons(this->port);
	if(connect(sock, (struct sockaddr*) &server, sizeof(server)) == -1){
		throw "Error while connecting socket.\n";
		return -3;
	}

	int hs = ics_handshake();
	if(hs < 0){
		printf("Error code: %d\n", hs*(-1));
		throw "Handshake error";
	}

	return 0;
}

int ics_server::ics_getinfo(){
	int p;
	std::string a;
	std::cout << "Input server IP(ICS uses IPv6!):\n";
	std::cin >> a;
	std::cout << "Specify server port:\n";
	std::cin >> p;
	if (p > 65535 || p <= 0){
		std::cout << "Invalid port number!\n";
		return -1;
	}
	this->addr = a;
	this->port = p;
	return 0;
}

std::string ics_server::ics_clist(){
	std::string clist;
	msg = CL_LIST_REQ;
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);
	if(ics_recv(512, SRV_LIST_RESP) != 0)
		return "";
	clist = buf;
	msg = CL_LIST_ACC;
       msg.append(";");       //czy to jest potrzebne
	send(sock, msg.c_str(), msg.length(), 0);
	return clist;
}

int ics_server::ics_disconnect(){
	std::ofstream ssid_file;
	msg = CL_END_REQ;
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);
	if(ics_recv(3, SRV_END_ACC) != 0)
		return -1;
	ssid_file.open(dirpath + "ssid", std::ofstream::out | std::ofstream::trunc); // Czyszczenie pliku ./.ics/ssid
	ssid_file.close();
	return 0;
}
