/*
 * ICS Client Class File
 *
 * Autorzy: Bartlomiej Partyka, Michal Urbanski
 *
 * Data utworzenia: 10/04/2020
 */

#include "ics.h"
ics_server::ics_server()
{

	//mutex
	mutex = PTHREAD_MUTEX_INITIALIZER;

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
	name = "ICS_User";
}
/*
 * Kiedy ics_recv jest wywolane, zaklada ze w zmiennej msg znajduje sie
 * poprzedni komunikat. Dzieki temu jest w stanie ponowic zapytanie w razie
 * blednego odczytu.
 */
int ics_server::ics_recv(int len, std::string flag)
{
	char temp[len+1];
	char *check = new char;
	char errorf[] = ERROR;
	for(;;){
		pthread_mutex_lock(&mutex);
		recv(sock, check, 1, MSG_PEEK);
		if(*check == '3'){
			pthread_mutex_unlock(&mutex);
			continue;
		}
		int numbytes = recv(sock, temp, len, 0);
		if (numbytes == -1){
			std::cout << "Recv function error: " << errno << '\n';
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		if(strncmp(temp, errorf, 2) ==  0){
			std::cout << "Server error, aborting.\n";
			pthread_mutex_unlock(&mutex);
			return -2;
		}
		temp[len] = '\0';
		if(strncmp(temp, flag.c_str(), 2) != 0)
		{
			send(sock, msg.c_str(), msg.length(), 0);
			pthread_mutex_unlock(&mutex);
			continue;
		}
		else
		{
			buf = temp + 3;
			buf.pop_back(); //ends with ';'
			pthread_mutex_unlock(&mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&mutex);
	return -3;
}
/*
 * Procedura handshake, przeprowadzona krok po kroku.
 */
int ics_server::ics_handshake()
{	
	std::string pass;
	std::string ssid;
	
	std::cout << "Attempting handshake...\n";

	msg = CL_CONNECTION_REQ + semi;

	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(3 + CH_LEN + 1, SRV_CHALLENGE_REQ) != 0)
		return -2;

	msg = CL_CHALLENGE_RESP + semi;

	std::cout << "Server password:";

	pass = "password;";
	msg.append(buf + semi);
	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(5, SRV_CHALLENGE_ACC) != 0)
		return -4;

	if(buf != "0")
		return -3; //incorrect password
	std::cout << "Password OK\n";
	
	msg = CL_SSID_REQ + semi;

	std::ifstream ssid_file(dirpath + "ssid");
	ssid_file >> ssid;
	ssid_file.close();
	if(ssid.length() != 3){
		std::cout << "No SSID found. Requesting new SSID...\n";
		ssid = "0";
	}

	msg.append(ssid + semi);

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
	msg = CL_NAME + semi;

	std::string nickname = this->name;
	msg.append(nickname + semi);

	std::cout << "Introducing client...\n";

	send(sock, msg.c_str(), msg.length(), 0);
	
	if(ics_recv(3, SRV_NAME_ACC) != 0)
		return -9;
	return 0;
}
int ics_server::ics_connect()
{
	/*
	 * Uzyskujemy IP z string address, w formie d:d:d:d:d:d:d:d
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
		return -4;
	}
	if(pthread_create(&receiver, NULL, &ics_server::ics_listen_helper, this)){
		throw "Error creating receiver thread!";
		return -5;
	}

	return 0;
}

/*
 * Zapisanie adresu i portu w klasie klienta. 
 *
 * Moze warto zapisywac je rowniez do pliku zeby latwiej wznawiac komunikacje?
 */

int ics_server::ics_getinfo(std::string a, int p){
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
	msg = CL_LIST_REQ + semi;
	send(sock, msg.c_str(), msg.length(), 0);
	if(ics_recv(512, SRV_LIST_RESP) != 0)
		return "";
	clist = buf;
	msg = CL_LIST_ACC + semi;       //TODO Usunac komunikat 43. W niczym nie pomaga i jest zbedny.
	send(sock, msg.c_str(), msg.length(), 0);
	return clist;
}

/*
 * Prosba do serwera o usuniecie nas z listy klientow.
 */

int ics_server::ics_disconnect(){
	std::cout << "Attempting to disconnect...\n";
	std::ofstream ssid_file;
	msg = CL_END_REQ + semi;
	send(sock, msg.c_str(), msg.length(), 0);
	if(ics_recv(3, SRV_END_ACC) != 0){
		std::cout << "Warning, no response from server.\nClient might still be registered in the server itself.\n";
		return -1;
	}
	ssid_file.open(dirpath + "ssid", std::ofstream::out | std::ofstream::trunc); // Czyszczenie pliku ./.ics/ssid
	ssid_file.close();
	std::cout << "SSID Removed\n";
	std::cout << "Waiting for  receiver...\n";
	pthread_join(receiver, NULL);
	std::cout << "Exiting...\n";
	return 0;
}
/*
 * Funkcja pomocnicza do wysylania pliku. Dziala podobnie do ics_recv ale uzywa wiekszych buforow danych.
 */
int ics_server::ics_sfile_recv(std::string flag, char* fbuffer, size_t sendsize){
	char temp[5];
	char* check = new char;
	char errorf[] = ERROR;
	for(;;){
		pthread_mutex_lock(&mutex);
		recv(sock, check, 1, MSG_PEEK);
		if(*check == '3'){
			pthread_mutex_unlock(&mutex);
			continue;
		}
		if(flag == SRV_UP_PAYLOAD_ACC){
			temp[5] = '\0';
			int numbytes = recv(sock, temp, 5, 0);
			if(numbytes == -1){
				std::cout << "Recv function error: " << errno << std::endl;
				pthread_mutex_unlock(&mutex);
				return -1;
			}
			if(strncmp(temp, errorf, 2) == 0){
				std::cout << "Server error, aborting.\n";
				pthread_mutex_unlock(&mutex);
				return -2;
			}else if(strncmp(temp, flag.c_str(), 2) != 0){
				send(sock, fbuffer, sendsize, 0);
				pthread_mutex_unlock(&mutex);
				continue;
			}else{
				buf = temp + 3;
				buf.pop_back(); //ends with ;
				pthread_mutex_unlock(&mutex);
				return 0;
			}
		}else if(flag == SRV_UP_COMPLETE){
			temp[3] = '\n';
			int numbytes = recv(sock, temp, 3, 0);
			if(numbytes == -1){
				std::cout << "Recv function error: " << errno << std::endl;
				pthread_mutex_unlock(&mutex);
				return -1;
			}
			if(strncmp(temp, errorf, 2) == 0){
				std::cout << "Server error, aborting.\n";
				pthread_mutex_unlock(&mutex);
				return -2;
			}else if(strncmp(temp, flag.c_str(), 2) != 0){
				send(sock, fbuffer, sendsize, 0);
				pthread_mutex_unlock(&mutex);
				continue;
			}else{
				buf = temp + 3;
				buf.pop_back();
				pthread_mutex_unlock(&mutex);
				return 0;
			}
		}else{
			pthread_mutex_unlock(&mutex);
			return -3; //Wrong use of the function;
		}
	}
}

/*
 * Procedura wysylania pliku. Najpierw sprawdzamy czy istnieje i przesylamy parametry do serwera, a nastepnie czytamy z pliku i wysylamy segmenty danych.
 */

int ics_server::ics_send(std::string user, std::string path_to_file, int blocksize){
	std::ifstream file;
	int len, segments;
	char* fbuffer = new char[blocksize+5];
	//25;n; + payload
	strcpy(fbuffer, "25;"); //CL_UP_PAYLOAD

	file.open(path_to_file, std::ifstream::binary);
	if(!file.is_open()){
		std::cout << "File not found!\n";
		return -1;
	}

	file.seekg(0, file.end);
	len = file.tellg();
	file.seekg(0, file.beg);

	int pos = path_to_file.find_last_of('/');
	std::string filename = path_to_file.substr(pos+1);

	//parametry dla odbiorcow
	msg = CL_UPLOAD_REQ + semi + user + semi + filename + semi + std::to_string(len) + semi;

	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(3, SRV_UPLOAD_ACC) != 0){
		std::cout << "No response from server\n";
		return -2;
	}
	//serwer ostatecznie decyduje o liczbie segmentow (zalezy od konfiguracji)
	msg = CL_UPLOAD_SEG_SIZE + semi + std::to_string(blocksize) + semi;


	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(3, SRV_UPLOAD_SEG_NUM) != 0){
		std::cout << "Failed to retrieve number of segments from server\n";
		return -3;
	}

	segments = atoi(buf.c_str());

	if(ics_recv(5, SRV_RECEIVER_INFO) != 0){
		std::cout << "Did not get response from target user. Aborting...\n";
		return -4;
	}
	if(buf != "Y"){
		std::cout << "User " << user << " declined file transfer.\n";
		return 0;
	}
	bool received = true;
	size_t count;
	for(;;){
		if(received){
			std::string num = std::to_string(segments) + semi;
			strcpy(fbuffer+3, num.c_str());
			file.read(fbuffer+5, blocksize);
			count = file.gcount();
		}
		send(sock, fbuffer, count, 0);
		if(segments != 1){
			if(ics_sfile_recv(SRV_UP_PAYLOAD_ACC, fbuffer, count) != 0){
				std::cout << "Error while sending file!\n";
				return -5;
			}
			if(std::stoi(buf) == segments){
				received = true;
				--segments;
			}else{
				received = false;
			}
		}else{
			if(ics_sfile_recv(SRV_UP_COMPLETE, fbuffer, count) != 0){
				std::cout << "Error while sending final segment!\n";
				return -6;
			}else{
				std::cout << "File upload complete!\n";
				break;
			}
		}
		
	}

	return 0;
}

/*
 * Glowna petla watku sluchajacego. Reaguje tylko na sygnal rozlaczenia i odebrania pliku
 */

void* ics_server::ics_listen(){
	int r;
	char temp[2];
	std::string requestf = SRV_DOWNLOAD_REQ;
	std::string shutdown = SRV_END_ACC;
	for(;;){
		pthread_mutex_lock(&mutex);
		recv(sock, temp, 2, MSG_PEEK);
		if(strcmp(temp, requestf.c_str()) == 0){
			r = ics_recv_file();
			if(r != 0)
				std::cout << "Error code: " << r << "\nError while receiving file!\n";
		}
		if(strcmp(temp, shutdown.c_str()) == 0){
			pthread_mutex_unlock(&mutex);
			return NULL;
		}
		pthread_mutex_unlock(&mutex);
	}
//can't end up here
return NULL;
}

/*
 * Watek musi wiedziec o naszej instancji ics_server
 */

void* ics_server::ics_listen_helper(void* context){
	return ((ics_server *)context)->ics_listen();
}

/*
 * Funkcja zajmujaca sie komunikacja dla odbierania pliku. Ma inne wymagania niz wersje dla wysylania i dla zwyklej komunikacji
 */

int ics_server::ics_rfile_recv(char *fbuffer, size_t size, int* received, int segments){
	char temp[size + 6];
	char errorf[] = ERROR;
	char flag[] = SRV_DOWN_PAYLOAD;
	for(;;){
		*received = recv(sock, temp, size + 5, 0);
		if(*received < 0){
			std::cout << "Recv function error while downloading file.\n";
			return -1;
		}
		if(strncmp(temp, errorf, 2) == 0){
			std::cout << "Server error while downloading.\n";
			return -2;
		}else if(strncmp(temp, flag, 2) == 0){
			char s = temp[4];
			int seg = s - '0';	//brzydkie, tymczasowe rozwiazanie do testow(segmenty < 10).
			if(seg != segments){
				send(sock, msg.c_str(), msg.length(), 0);
				continue;
			}
			strncpy(fbuffer, temp + 5, *received - 5);
			if(segments == 1)
				return 1;
			else
				return 0;
		}
	}

}

int ics_server::ics_recv_file(){
	char* initial_msg = new char[1024];
	recv(sock, initial_msg, 1024, 0);
	std::string initial_message(initial_msg + 3);

	//Get file details

	std::string name, filename, size, answer;
	int pos = initial_message.find_first_of(';');
	name = initial_message.substr(0, pos);
	filename = initial_message.substr(pos+1);
	pos = filename.find_first_of(';');
	size = filename.substr(pos+1);
	filename = filename.substr(0, pos);

	//Tell the user

	std::cout << "Incoming file \"" << filename << "\", size " << size  << " from user " << name << std::endl;
	std::cout << "Do you wish to accept? (y/N)\n";
	std::cin >> answer;
	if(answer != "y"){
		std::cout << "Aborting...\n";
		msg = CL_DOWNLOAD_RESP + semi + 'N' + semi;
		send(sock, msg.c_str(), 5, 0);
		return 0;
	}
	std::cout << "Select segment size (size of a single packet): ";
	std::string seg_size;
	std::cin >> seg_size;

	msg = CL_DOWNLOAD_RESP + semi + 'Y' + semi + seg_size + semi;
	send(sock, msg.c_str(), msg.length(), 0);

	if(ics_recv(7, SRV_DOWNLOAD_SEG_NUM) != 0){
		return -1;
	}
	int segments;
	segments = std::stoi(buf);

	//prepare local file

	std::ofstream file;
	file.open(dirpath+filename, std::ios::app);
	if(!file.is_open()){
		std::cout << "Failed to create file " << filename << std::endl;
		msg = ERROR + semi;
		send(sock, msg.c_str(), msg.length(), 0);
		return -2;
	}
	
	msg = CL_DOWNLOAD_READY + semi;
	send(sock, msg.c_str(), msg.length(), 0);
	char* fbuffer = new char[std::stoi(seg_size) + 1];
	for(;;){
		int *received = NULL;
		int r = ics_rfile_recv(fbuffer, std::stoi(seg_size), received, segments);
		if(r < 0){
			std::cout << "Error while retreiving segment " << segments <<  " data.\n";
			return -3;
		}
		if(*received > std::stoi(seg_size)){
			msg = ERROR + semi;
		}else{
			file.write(fbuffer, *received);
			msg = CL_DOWN_ACC + semi + std::to_string(segments) + semi;
			--segments;
		}
		send(sock, msg.c_str(), msg.length(), 0);
		if(r == 1){
			file.close();
			msg = CL_DOWN_COMPLETE + semi;
			send(sock, msg.c_str(), msg.length(), 0);
			break;
		}
	}
	return 0;
}

/*
 * vvv Funkcje pomocnicze dla idc_input_handler
 */

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
 * Wywolujemy funkcje ics_server w zaleznosci od wpisanej komendy.
 * Pierwszy pomysl uzywal switch ale byly trudnosci z funkcja zamieniajaca string na unikalny int. Dlatego funkcja jest dosyc brzydka, moze uda sie to naprawic w przyszlosci.
 */

int ics_input_handler::execute(){

	if(command == "name" && !this->connected){
		if(args == "") {
			std::cout << "Current name: " << sr->ics_getname() << std::endl;;
			return 0;
		}else{
			if(args.find_first_of(';') != std::string::npos){
				std::cout << "';' is not allowed as a part of the name.\n";
				return 0;
			}
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
		try{
			sr->ics_connect();
		}catch(const char* err){
			std::cout << "While connecting: " << err << std::endl;
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
	}else if(command == "send" && this->connected){
		std::string user, file, segsize;
		int pos = args.find_first_of(' ');
		user = args.substr(0, pos);
		file = args.substr(pos+1);
		pos = file.find_first_of(' ');
		segsize = file.substr(pos + 1);
		file = file.substr(0, pos);

		if(sr->ics_send(user, file, std::stoi(segsize)) < 0)
			return -1;
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
	std::cout << ">> ";
	std::getline(std::cin, input);

	int pos = input.find_first_of(" ");

	command = input.substr(0, pos);
	args = input.substr(pos+1);
	if(args==command)
		args = "";

	return execute();
}

/*
 * Help text
 */

void ics_input_handler::print_help(){
	std::cout <<
R"(
Basic syntax for commands: <command> <arguments>.
Commands are case sensitive. [arguments] are optional.

Current command list:
name [new_name] - Prints your current name.
	Changes name to [new_name] if given, default is ICS_User.
set <address> [port] - Set up connection parameters.
	You need to set the parameters of the server before connecting.
	Remember that ICS uses IPv6. The default port is 45456.
connect - Connect to a server using given parameters.
help - Show this help message.
quit - Quit this client app.

While connected:
list - List the names of clients currently available on the server.
send <user> <file> <seg size> - send <file> to <user> connected to the server.
	Segment size specifies the size of packets used for uploading the file.
disconnect - Willingly disconnect from the server

)";
return;
}
