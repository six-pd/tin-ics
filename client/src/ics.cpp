#include "ics.h"
ics_server::ics_server()
{
	/*
	 * Tworzymy gniazdo
	 */
	std::cout << "Initializing socket...";
	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if(sock == -1){
		std::cout << "Failed!\n";
		throw "Failed to create IPv6 socket!\n";
	}
	else
		std::cout << "Success\n";
	/*
	 * IPv6
	 */
	server.sin6_family = AF_INET6;
	client.sin6_family = AF_INET6;
	client.sin6_addr = in6addr_any;
	client.sin6_port = 0;
	
	std::cout << "Binding...";
	if(bind(sock, (struct sockaddr*) &client, sizeof(client)) == -1){
		std::cout << "Failed!\n";
		throw "Failed to bind IPv6 socket!\n";
	}
	else
		std::cout << "Success\n";
	name = "ICS User";
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
	
	std::cout << "Attempting handshake...\n";

	msg = CL_CONNECTION_REQ;
	msg.append(";");
	//std::cout << msg << '\n';
	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(3 + CH_LEN + 1, SRV_CHALLENGE_REQ) != 0)
		return -2;

	msg = CL_CHALLENGE_RESP;
	msg.append(";");
	std::cout << "Server password:";

	pass = "password;";
	msg.append(buf);
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(5, SRV_CHALLENGE_ACC) != 0)
		return -4;

	if(buf != "0")
		return -3; //incorrect password
	std::cout << "Password OK\n";
	
	msg = CL_SSID_REQ;
	msg.append(";");

	std::ifstream ssid_file(dirpath + "ssid");
	if(!ssid_file.is_open())
		return -5;
	ssid_file >> ssid;
	ssid_file.close();
	if(ssid.length() != 3){
		std::cout << "No SSID found. Requesting new SSID...\n";
		ssid = "0";
	}

	msg.append(ssid);
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);

	if(ssid == "0"){
		if(ics_recv(7, SRV_NEW_SSID) != 0)
			return -6;
		std::ofstream ssid_file(dirpath + "ssid");
		ssid_file << buf;
		if(ssid_file.fail())
			std::cout << "Failed to save SSID!\n";
	ssid_file.close();
	}else{
		if(ics_recv(3, SRV_SSID_ACC) != 0)
			return -7;
	}
	msg = CL_NAME;
	msg.append(";");
	std::string nickname = this->name;
	nickname.append(";");
	msg.append(nickname);

	std::cout << "Introducing client...\n";

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
	std::cout << "Starting connection...\n";
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
	std::cout << "Socket connected...\n";
	int hs = ics_handshake();
	if(hs < 0){
		printf("Error code: %d\n", hs*(-1));
		throw "Handshake error";
	}

	return 0;
}

int ics_server::ics_getinfo(std::string a, int p){
	/*int p;
	std::string a;
	std::cout << "Input server IP(ICS uses IPv6!):\n";
	std::cin >> a;
	std::cout << "Specify server port:\n";
	std::cin >> p;*/
	std::cout << "Address: " << a << "\nPort: " << p << std::endl;
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
	std::cout << "Attempting to disconnect...\n";
	std::ofstream ssid_file;
	msg = CL_END_REQ;
	msg.append(";");
	send(sock, msg.c_str(), msg.length(), 0);
	if(ics_recv(3, SRV_END_ACC) != 0){
		std::cout << "Warning, no response from server.\nClient might still be registered in the server itself.\n";
		return -1;
	}
	ssid_file.open(dirpath + "ssid", std::ofstream::out | std::ofstream::trunc); // Czyszczenie pliku ./.ics/ssid
	ssid_file.close();
	std::cout << "SSID Removed, client disconnected.\n";
	return 0;
}

void ics_server::ics_setname(std::string nm){
	name = nm;
	return;
}

std::string ics_server::ics_getname(){
	return this->name;
}

/*Input handler*/

ics_input_handler::ics_input_handler(ics_server* serv){
	std::cout << "Starting input handler...\n";
	sr = serv;
	set = false;
	first_start = true;
	connected = false;
}


/*
 * Wywolujemy funkcje ics_server w zaleznosci od wpisanej komendy
 */

int ics_input_handler::execute(){

	if(command == "name" && !this->connected){
		if(args == "") {
			std::cout << "Current name: " << sr->ics_getname() << std::endl;;
			return 0;
		}else{
			sr->ics_setname(args);
			std::cout << "Name set.\n";
			return 0;
		}
	}else if(command == "set" && !this->connected){
		std::string address, ps;
		int port;
		int p = args.find_first_of(' ');
		address = args.substr(0,p);
		ps = args.substr(p+1);
		port = atoi(ps.c_str());
		if(port==0){
			port = 45456;
			std::cout << "No port given. Defaulting to 45456...\n";
		}
		if(sr->ics_getinfo(address, port) == -1)
			return -1;
		this->set = true;
		std::cout << "Connection parameters set!\n";
		return 0;
	}else if(command == "connect" && !this->connected){
		if(!this->set){
			std::cout << "Specify connection parameters\n";
			return -1;
		}
		int ret;
		try{
			ret = sr->ics_connect();
		}catch(const char* err){
			std::cout << "While connecting: " << err << "Function returned " << ret << std::endl;
			return -2;
		}
		this->connected = true;
		std::cout << "Connection successfull!\n";
		return 0;
	}else if(command == "disconnect" && this->connected){
		sr->ics_disconnect();
		this->connected = false;
		return 0;
	}else if(command == "list" && this->connected){
		std::cout << "Client list:\n" << sr->ics_clist() << std::endl;
		return 0;
	}else if(command == "quit"){
		std::cout << "Shutting down client...\n";
		return 45456;
	}else if(command == "help"){
		print_help();
		return 0;
	}else{
		std::cout << "Command unknowm. Type \"help\" for a list of commands.\n";
		return 0;
	}
	return -1; //can't end up here
}

/*
 * Prosty sposob aby pozyskac input
 */

int ics_input_handler::get_command(){
	std::string input;
	if(first_start){
		first_start=false;
		std::cout << "Input handler ready.\nWelcome to ICS. Type \"help\" for available commands.\n";
	}
	std::cout << ">";
	std::getline(std::cin, input);

	int pos = input.find_first_of(" ");

	command = input.substr(0, pos);
	args = input.substr(pos+1);
	if(args==command)
		args = "";

	return execute();
}

void ics_input_handler::print_help(){
	std::cout << "Basic syntax for commands: <command> <arguments>. Commands are case sensitive.\nCurrent command list:\nname (optional) <new_name> - Prints your current name. Changes name to new_name if it's given.\nset <address> (optional) <port> - Set up connection parameters. You need to set the parameters of the server before making a connection. Remember that ICS uses IPv6. The default port is 45456.\nconnect - Connect to a server using given parameters.\nhelp - Show this text.\n\nWhile connected:\nlist - List the names of clients currently available on the server.\ndisconnect - Willingly disconnect from the server\n";
return;
}
