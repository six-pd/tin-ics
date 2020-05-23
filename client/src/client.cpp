#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ics.h"

ics_server ics_serv;

int main(){
	int cont = 0;
	std::string dummy;
	std::cout << "Welcome to ICS!\n";

	for(;;){
		cont = ics_serv.ics_getinfo();
		if (cont < 0)
			continue;
		try{
			cont = ics_serv.ics_connect();
		}catch(const char* err){
			std::cout << "While connecting: " << err << '\n';
			continue;
		}
		if (cont == 0)
			break;
		//std::cout << cont << '\n';

	}
	std::cout <<"Connection successful.\n";

	std::cout << "Client list:\n" << ics_serv.ics_clist() << '\n';

	std::cout << "Press Enter to disconnect\n";

	std::cin >> dummy;

	ics_serv.ics_disconnect();

	return 0;
}
